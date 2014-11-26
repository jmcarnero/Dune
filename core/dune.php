<?php
/**
 * Inicializador
 *
 * En el directorio de modulos (directorio "mods"):
 * - la vista tiene el nombre del modulo y es obligatoria, ej: portada.php
 * - el controlador (si existe) ha de ser hijo de la clase "core/class.controlazo.php" y tiene el nombre del modulo con sufijo "_contr", ej: portada_contr.php
 *   la clase model puede estar en el mismo fichero que la clase controlador, pero no al reves
 * - el modelo (si existe) ha de ser hijo de "core/class.modelazo.php" y tiene el nombre del modulo con sufijo "_model", ej: portada_model.php
 * URL: http://dominio.tld/directorio/?modulo=metodo&parametro=valor[...]
 * - dominio.tld -> es el dominio donde corre la aplicacion
 * - directorio -> de momento no tiene mas uso que ser el directorio donde se encuentra Dune (en caso de que no este en el raiz)
 * - modulo -> el modulo a cargar; si se omite se cargara el modulo por defecto (portada), si no se encuentra se cargara el modulo de error (si no se encuentra se envia un mensaje de aviso)
 * - metodo -> metodo a cargar del modulo pedido; se puede omitir, si no existe se ignora
 * - parametro=valor -> cualquier numero de parametros con o sin valor, en el formato ordinario de una URL (separados por &); de momento se recogen por post, no se pasan como parametros al metodo llamado
 *
 * @since 2014-11-23
 * @author jm_carnero@sargazos.net
 */
class Dune {

	/**
	 * Rutas a los directorios del framework; si es necesario cambiar alguna debera hacerse aqui mismo
	 * - core: clases core
	 * - libs: librerias
	 * - mods: modulos, directorio para los ficheros de vistas, controladores y modelos de cada modulo
	 * - tpl: plantillas, ficheros con extension ".tpl"
	 *
	 * @var array
	 * @access protected
	 */
	protected $aDirectorios = array('core' => 'core/', 'libs' => 'incs/', 'mods' => 'mods/', 'tpl' => 'tpl/');

	/**
	 * Nombres de ficheros de configuracion, plantilla base, etc.
	 * - config: fichero de configuracion
	 * - plantilla: fichero base de plantilla
	 * - sesion: controlador de sesion //TODO de momento no integrado como objeto
	 *
	 * @var array
	 * @access protected
	 */
	protected $aFicheros = array('config' => 'config.php', 'plantilla' => 'dune.tpl', 'sesion' => 'sesion.php');

	protected $sContenidos = false; //contenidos cargados de la vista/controlador/modelo
	protected $oControlazo = false; //instancia de la clase controlador
	private $sModulo = 'error'; //nombre del modulo a cargar

	function __construct(){
		Dune::baseDir();

		//includes comunes
		require(D_BASE_DIR.$this->aDirectorios['libs'].$this->aFicheros['config']); //parametros basicos de la aplicacion
		$this->setDebug();
		require(D_BASE_DIR.$this->aDirectorios['libs'].$this->aFicheros['sesion']); //variables e inicio de sesion

		$this->ipCliente();
		$this->tempDir();

		define('LOGOUT', D_BASE_URL.'?logout');

		/* final */
		$this->pintaPagina();
	}

	/**
	 * Crea las constantes BASE_DIR y BASE_URL,
	 * por esto la clase init debe estar en el directorio raiz
	 */
	public static function baseDir(){
		if(!defined('D_BASE_DIR')) define('D_BASE_DIR', str_replace('\\', '/', realpath(dirname(__FILE__).'/..')).'/');
		if(!defined('D_BASE_URL')){
			$cadBase = array_pop(explode('/', trim(D_BASE_DIR, '/')));
			$phpSelf = trim(dirname($_SERVER['PHP_SELF']), '/');
			$cadBase = substr($phpSelf, strpos($phpSelf, $cadBase) + strlen($cadBase));
			$cadBase = $cadBase === false?array():explode('/', trim($cadBase, '/'));
			define('D_BASE_URL', str_repeat('../', count($cadBase)));
		}
	}

	/**
	 * Carga el controlador relacionado con el modulo,
	 * deja una instancia del controlador en $this->oControlazo
	 */
	private function cargaControlador(){
		//clases base de controlador y modelo
		require(D_BASE_DIR.$this->aDirectorios['core'].'class.controlazo.php');
		require(D_BASE_DIR.$this->aDirectorios['core'].'class.modelazo.php');

		//intenta cargar el controlador (puede contener tambien el modelo)
		if(is_readable(D_BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.D_SUFIJO_CONTROLADOR.'.php')){
			include(D_BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.D_SUFIJO_CONTROLADOR.'.php');
		}

		//carga el controlador si existe
		$aux = ucfirst(strtolower($this->sModulo));
		if(class_exists($aux)){
			$this->oControlazo = new $aux();
			//TODO llamar a un metodo pedido en la URL?
		}

	}

	/**
	 * Verifica si existe el modulo indicado en la URL,
	 * si no existe busca el modulo de error,
	 * si tampoco existe avisa con un mensaje
	 */
	private function buscaModulo(){
		$sId = ''; //valor de la id para la seccion solicitada

		//seleccion de modulo, portada por defecto
		$this->sModulo = empty($_GET)?'portada':key($_GET); //nombre del modulo

		if(in_array($this->sModulo.'.php', array_map('basename', glob(D_BASE_DIR.$this->aDirectorios['mods'].'*.php')))){ //todo los modulos que se encuentran en el directorio "mods"
			if(isset($_GET[$this->sModulo])) $sId = (string)$_GET[$this->sModulo];
		}
		else{ //portada o seccion desconocida
			if(empty($_SERVER['QUERY_STRING']) || isset($_GET['portada'])){
				$this->sModulo = 'portada';
			}
			elseif(isset($_GET['logout'])){
				$this->sModulo = 'login';
			}
			else{
				$this->sModulo = 'error';
			}
		}

		if(!is_readable(D_BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.'.php')){
			echo "Error fatal.\n\nNo se puede cargar el m&oacute;dulo [".$this->sModulo.'].'; //TODO cambiar esto por algo mas elaborado
			exit(0);
		}
	}

	/**
	 * Guarda la IP del cliente actual
	 */
	private function ipCliente(){
		//redes privadas ("10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16");
		$sIpCliente = ''; //ip desconocida

		if(!empty($_SERVER['HTTP_X_FORWARDED_FOR']))
			$sIpCliente = $_SERVER['HTTP_X_FORWARDED_FOR'];
		elseif(!empty($_SERVER['REMOTE_ADDR']))
			$sIpCliente = $_SERVER['REMOTE_ADDR'];

		define('D_IP_CLIENTE', $sIpCliente);
	}

	/**
	 * Compone la pagina y la envia al navegador
	 *
	 */
	private function pintaPagina(){
		if(!headers_sent($file, $line)){
			header('Content-Type: text/html; charset=UTF-8');
		}
		elseif(D_DEBUG){
			echo 'Headers ya enviados en ['.$file.'] en linea ['.$line;
		}

		/*construye pagina*/
		$this->buscaModulo();
		$this->cargaControlador();

		if($this->oControlazo === false){ //no hay controlador de contenidos, solo vista, los contenidos se cargan desde aqui
			ob_start();
			include(D_BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.'.php'); //carga del modulo

			$this->sContenidos = ob_get_contents(); //contenidos a mostrar procesados
			ob_end_clean();

			if(!empty($this->sContenidos)){
				$sContenidos = $this->sContenidos; //contenidos del modulo
			}
			require(D_BASE_DIR.$this->aDirectorios['tpl'].$this->aFicheros['plantilla']);
		}
		else{ //si hay controlador, la carga de contenidos se pasa al controlador
			$this->oControlazo->pintaPagina(D_BASE_DIR.$this->aDirectorios['tpl'].$this->aFicheros['plantilla']);
		}

		if(D_DEBUG){
			//echo('uso de traducciones:');print_r(l10n_sel()->getTraduccionUso(-1));
		}
	}

	/**
	 * Pone la aplicacion en modo debug o no
	 * en modo debug se muestran errores
	 */
	private function setDebug(){
		if(defined('D_DEBUG') && D_DEBUG){
			error_reporting(E_ALL);
			//error_reporting(E_ALL ^ E_NOTICE);
			ini_set('display_errors', '1');
		}
		else{
			error_reporting(0);
			ini_set('display_errors', '0');
			if(function_exists('xdebug_disable')){
				xdebug_disable();
			}
		}
	}

	/**
	 * Busca la ruta del directorio temporal
	 */
	private function tempDir(){
		$sTempDir = null;

		if(function_exists('sys_get_temp_dir')){
			$sTempDir = realpath(sys_get_temp_dir());
		}
		else{
			if($temp=getenv('TMP')) return $temp;
			if($temp=getenv('TEMP')) return $temp;
			if($temp=getenv('TMPDIR')) return $temp;
			$temp = tempnam(__FILE__, '');
			if(file_exists($temp)){
				unlink($temp);
				$sTempDir = dirname($temp);
			}
		}

		define('D_TEMP_PATH', $sTempDir);
	}

	//traduccion de textos
	//sustituye a la ce Controlazo cuando no hay controlador //TODO buscar una solucion mejor que tenerla duplicada
	protected function trad($cadena){
		if(function_exists('_tradR')){
			$cadena = _tradR($cadena);
		}

		return $cadena;
	}

}
