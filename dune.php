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

	function __construct(){
		$this->baseDir();

		//includes comunes
		require(BASE_DIR.'incs/config.php'); //parametros basicos de la aplicacion

		$this->setDebug();

		require(BASE_DIR.'incs/sesion.php'); //variables e inicio de sesion

		//internacionalizacion y localizacion de la pagina
		//LENGUAJE_DEFECTO
		include_once(BASE_DIR.'incs/class.l10n.inc');
		l10n_sel();
		_locale();
		//_trad('clave_test');

		//includes core
		require(BASE_DIR.'core/class.controlazo.php');
		require(BASE_DIR.'core/class.modelazo.php');

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
	private function baseDir(){
		if(!defined('BASE_DIR')) define('BASE_DIR', str_replace('\\', '/', dirname(__FILE__)).'/');
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
		/*carga de contenidos*/
		ob_start();
		include($sRutaModulo); //carga del modulo

		$sContenidos = ob_get_contents(); //contenidos a mostrar procesados
		ob_end_clean();
		/*carga de contenidos fin*/
	}

	/**
	 * Carga el controlador/modelo relacionado con el modulo
	 */
	private function cargaControladorModelo(){
		$oController = false; //instancia de la clase controlador
		$oModel = false; //instancia de la clase modelo

		if(is_readable(BASE_DIR.'mods/'.$sModulo.SUFIJO_CONTROLADOR.'.php')){
			include(BASE_DIR.'mods/'.$sModulo.SUFIJO_CONTROLADOR.'.php');

			$aux = ucfirst(strtolower($sModulo));
			if(class_exists($aux)){
				$oController = new $aux();

				//carga el modelo en caso de que se encuentre en el mismo fichero que el controlador
				if(class_exists($aux.'_model')){
					$aux = $aux.'_model';
					$oModel = new $aux();
				}
			}
			$aux = null;
			unset($aux);
		}
		if(empty($oModel) && is_readable(BASE_DIR.'mods/'.$sModulo.SUFIJO_MODELO.'.php')){ //intenta cargar el modelo si no ha podido en el paso anterior
			include(BASE_DIR.'mods/'.$sModulo.SUFIJO_MODELO.'.php');

			$aux = ucfirst(strtolower($sModulo.'_model'));
			if(class_exists($aux)){
				$oModel = new $aux();
			}
			$aux = null;
			unset($aux);
		}
	}

	/**
	 * Carga el modulo indicado en la URL
	 */
	private function cargaModulo(){
		$sId = ''; //valor de la id para la seccion solicitada

		//seleccion de modulo, portada por defecto
		$sModulo = empty($_GET)?'portada':key($_GET); //nombre del modulo
		$sRutaModulo; //

		if(in_array($sModulo.'.php', array_map('basename', glob(BASE_DIR."mods/*.php")))){ //todo los modulos que se encuentran en el directorio "mods"
			if(isset($_GET[$sModulo])) $sId = (string)$_GET[$sModulo];
			$sRutaModulo = BASE_DIR.'mods/'.$sModulo.'.php';
		}
		else{ //portada o seccion desconocida
			if(empty($_SERVER['QUERY_STRING']) || isset($_GET['portada'])){
				$sModulo = 'portada';
				$sRutaModulo = BASE_DIR.'mods/portada.php';
				$sId = isset($_GET['posts'])?(string)$_GET['portada']:false;
			}
			elseif(isset($_GET['logout'])){
				$sModulo = 'login';
				$sRutaModulo = BASE_DIR.'mods/login.php';
			}
			else{
				$sModulo = 'error';
				$sRutaModulo = BASE_DIR.'mods/error.php';
				$sId = '?';
			}
		}

		if(!is_readable($sRutaModulo)){
			$sModulo = $_GET['mod'] = 'error';
			$sRutaModulo = BASE_DIR.'mods/error.php';
			$sId = '-r';
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
		header('Content-Type: text/html; charset=UTF-8');

		require(BASE_DIR.'tpl/plantilla.tpl');

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
