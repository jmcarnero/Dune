<?php
/**
 * Inicializador
 *
 * En el directorio de modulos (directorio "mods"):
 * - la vista tiene el nombre del modulo y es obligatoria, ej: portada.php
 * - el controlador (si existe) ha de ser hijo de la clase "core/class.controlazo.php" y tiene el nombre del modulo con sufijo "_contr", ej: portada_contr.php
 *   la clase model puede estar en el mismo fichero que la clase controlador, pero no al reves
 * - el modelo (si existe) ha de ser hijo de "core/class.modelazo.php" y tiene el nombre del modulo con sufijo "_model", ej: portada_model.php
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
	 *
	 * @var array
	 * @access protected
	 */
	protected $aDirectorios = array('core' => 'core/', 'libs' => 'incs/', 'mods' => 'mods/');

	protected $sContenidos = false; //contenidos cargados de la vista/controlador/modelo
	protected $oControlador = false; //instancia de la clase controlador
	protected $oModelo = false; //instancia de la clase modelo
	private $sModulo = 'error'; //nombre del modulo a cargar

	function __construct(){
		Dune::baseDir();

		//includes comunes
		require(BASE_DIR.$this->aDirectorios['libs'].'config.php'); //parametros basicos de la aplicacion

		$this->setDebug();

		require(BASE_DIR.$this->aDirectorios['libs'].'sesion.php'); //variables e inicio de sesion

		//internacionalizacion y localizacion de la pagina
		//LENGUAJE_DEFECTO
		include_once(BASE_DIR.$this->aDirectorios['libs'].'class.l10n.inc');
		l10n_sel();
		_locale();
		//_trad('clave_test');

		//includes core
		require(BASE_DIR.$this->aDirectorios['core'].'class.controlazo.php');
		require(BASE_DIR.$this->aDirectorios['core'].'class.modelazo.php');

		$this->ipCliente();
		$this->tempDir();

		define('LOGOUT', BASE_URL.'?logout');

		/* variables globales */
		$sTitle = ''; //titulo de la pagina
		$sMetas = ''; //tags meta
		$sCss = ''; //CSS de la pagina
		$sJs = ''; //scripts javascript

		//mensajes devueltos por el procesado de formularios y otros eventos
		$sMensajes = false;
		if(!empty($_SESSION['mensajes'])){
			$sMensajes = $_SESSION['mensajes'];
		}
		/* variables globales fin */

		/*construye pagina*/
		$this->cargaModulo();
		$this->cargaControladorModelo();
		$this->cargaContenidos();

		/* final */
		$this->pintaPagina();
	}

	/**
	 * Crea las constantes BASE_DIR y BASE_URL,
	 * por esto la clase init debe estar en el directorio raiz
	 */
	public static function baseDir(){
		if(!defined('BASE_DIR')) define('BASE_DIR', str_replace('\\', '/', realpath(dirname(__FILE__).'/..')).'/');
		if(!defined('BASE_URL')){
			$cadBase = array_pop(explode('/', trim(BASE_DIR, '/')));
			$phpSelf = trim(dirname($_SERVER['PHP_SELF']), '/');
			$cadBase = substr($phpSelf, strpos($phpSelf, $cadBase) + strlen($cadBase));
			$cadBase = $cadBase === false?array():explode('/', trim($cadBase, '/'));
			define('BASE_URL', str_repeat('../', count($cadBase)));
		}
	}

	/**
	 * Carga los contenidos relacionados con el modulo
	 */
	private function cargaContenidos(){
		$this->oControlador;

		ob_start();
		include(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.'.php'); //carga del modulo

		$this->sContenidos = ob_get_contents(); //contenidos a mostrar procesados
		ob_end_clean();
	}

	/**
	 * Carga el controlador/modelo relacionado con el modulo,
	 * deja una instancia del modelo en $this->oModelo (solo debe ser usado por el controlador, asi que no se hace nada mas aqui con este objeto),
	 * deja una instancia del controlador en $this->oControlador
	 */
	private function cargaControladorModelo(){
		//intenta cargar el modelo
		if(empty($this->oModelo) && is_readable(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.SUFIJO_MODELO.'.php')){
			include(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.SUFIJO_MODELO.'.php');
		}
		//intenta cargar el controlador (puede contener tambien el modelo)
		if(is_readable(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.SUFIJO_CONTROLADOR.'.php')){
			include(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.SUFIJO_CONTROLADOR.'.php');
		}

		//carga el modelo si existe
		$aux = ucfirst(strtolower($this->sModulo.'_model'));
		if(class_exists($aux)){
			$this->oModelo = new $aux();
		}
		//carga el controlador si existe
		$aux = ucfirst(strtolower($this->sModulo));
		if(class_exists($aux)){
			$this->oControlador = new $aux();
			//TODO llamar a un metodo pedido en la URL?
		}

	}

	/**
	 * Verifica si existe el modulo indicado en la URL,
	 * si no existe busca el modulo de error,
	 * si tampoco existe avisa con un mensaje
	 */
	private function cargaModulo(){
		$sId = ''; //valor de la id para la seccion solicitada

		//seleccion de modulo, portada por defecto
		$this->sModulo = empty($_GET)?'portada':key($_GET); //nombre del modulo

		if(in_array($this->sModulo.'.php', array_map('basename', glob(BASE_DIR.$this->aDirectorios['mods'].'*.php')))){ //todo los modulos que se encuentran en el directorio "mods"
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

		if(!is_readable(BASE_DIR.$this->aDirectorios['mods'].$this->sModulo.'.php')){
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

		define('IP_CLIENTE', $sIpCliente);
	}

	/**
	 * Compone la pagina y la envia al navegador
	 */
	private function pintaPagina(){
		if(!headers_sent($file, $line)){
			header('Content-Type: text/html; charset=UTF-8');
		}
		elseif(DEBUG){
			echo 'Headers ya enviados en ['.$file.'] en linea ['.$line;
		}

		if(!empty($this->sContenidos)){
			$aDatos = $this->oControlador->getDatos();
			if(!empty($aDatos)){
				foreach($aDatos as $clave => $valor){
					$$clave = $valor; //cada clave del array de datos se podra usar como una variable directa en la plantilla, igual que los contenidos obtenidos dl modulo (abajo, $sContenidos)
				}
			}
			$sContenidos = $this->sContenidos; //contenidos del modulo
		}
		require(BASE_DIR.'tpl/dune.tpl');

		if(DEBUG){
			echo('uso de traducciones:');print_r(l10n_sel()->getTraduccionUso(-1));
		}
	}

	/**
	 * Pone la aplicacion en modo debug o no
	 * en modo debug se muestran errores
	 */
	private function setDebug(){
		if(defined('DEBUG') && DEBUG){
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

		define('TEMP_PATH', $sTempDir);
	}

}
