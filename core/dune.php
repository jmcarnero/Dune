<?php
# Copyright (C) 2014 José M. Carnero <jm_carnero@sargazos.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# http://www.gnu.org/copyleft/gpl.html

//namespace Dune;

//constantes de core
//son necesarias aqui o en algun lugar donde esta clase las tenga inicializadas
defined('D_DIR_CORE') or define('D_DIR_CORE', 'core/'); //clases core
defined('D_DIR_CONFIG') or define('D_DIR_CONFIG', 'config/'); //directorio de ficheros de configuracion
defined('D_DIR_CONTROL') or define('D_DIR_CONTROL', 'mods/controladores/'); //modulos, directorio para los ficheros de controladores
defined('D_DIR_LIBS') or define('D_DIR_LIBS', 'incs/'); //librerias
defined('D_DIR_MODEL') or define('D_DIR_MODEL', 'mods/modelos/'); //modulos, directorio para los ficheros de modelos de cada modulo
defined('D_DIR_TPL') or define('D_DIR_TPL', 'tpl/'); //plantillas, ficheros con extension ".tpl"
defined('D_DIR_VISTA') or define('D_DIR_VISTA', 'mods/vistas/'); //modulos, directorio para los ficheros de vistas

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
 * - modulo -> el modulo a cargar; si se omite se cargara el modulo por defecto (D_MODULO_INICIO), si no se encuentra se cargara el modulo D_MODULO_ERROR (si no se encuentra se envia un mensaje de aviso)
 * - metodo -> metodo a cargar del modulo pedido; se puede omitir (se intentara cargar D_METODO_INICIO), si no existe se ignora
 * - parametro=valor -> cualquier numero de parametros con o sin valor, en el formato ordinario de una URL (separados por &); pueden recogerse con $_GET, o como parametros del metodo llamado (han de estar declarados en la firma del metodo)
 *
 * PHP 5 >= 5.1.2
 * [spl_autoload]
 *
 * @author José M. Carnero
 * @since 2014-11-23
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Dune {

	protected $bVista = false; //si true se esta cargando una vista
	protected $bControlador = false; //si true se esta cargando un controlador

	protected $sContenidos = false; //contenidos cargados de la vista/controlador/modelo
	protected $oControlazo = null; //instancia de la clase controlador

	protected $oRestazo = null; //instancia de la clase del controlador REST

	/**
	 * Nombres de ficheros de configuracion, plantilla base, etc.
	 * - config: fichero de configuracion
	 * - database: fichero de configuracion de base de datos //TODO soportar multiples conexiones
	 * - plantilla: fichero base de plantilla
	 * - sesion: controlador de sesion //TODO de momento no integrado como objeto
	 *
	 * @var array
	 */
	protected $aFicheros = array(
		'config' => 'config.php',
		'database' => 'database.php',
		'plantilla' => 'dune',
		'sesion' => 'sesion.php'
	);

	private $sModulo = 'error'; //nombre del modulo a cargar

	/**
	 * Constructor
	 *
	 * @todo controlar la existencia y contenido de las constantes (D_BASE_DIR, ...)
	 */
	public function __construct(){
		spl_autoload_register(array('Dune', 'autoload')); //autocarga de clases en demanda

		$this->baseDir();

		//includes comunes
		require D_BASE_DIR . D_DIR_CONFIG . $this->aFicheros['config']; //parametros basicos de la aplicacion
		require D_BASE_DIR . D_DIR_CONFIG . $this->aFicheros['database']; //parametros de base de datos
		$this->setDebug();
		require D_BASE_DIR . D_DIR_CONFIG . $this->aFicheros['sesion']; //variables e inicio de sesion

		defined('D_MODULO_INICIO') or define('D_MODULO_INICIO', 'portada'); //modulo a cargar en inicio, ya sea nombre de vista o de controlador
		defined('D_METODO_INICIO') or define('D_METODO_INICIO', 'inicio'); //metodo a cargar por omision, si no se pasa ?modulo=metodo
		defined('D_MODULO_ERROR') or define('D_MODULO_ERROR', 'error'); //modulo a cargar en error

		//gestion de errores
		if(D_DEBUG){
			include D_BASE_DIR . D_DIR_CORE . 'class.errorHandler.inc';
			//logFile('errors.err');
		}

		$this->ipCliente();
		$this->tempDir();

		/* final */
		$this->pintaPagina();
	}

	/**
	 * Carga automaticamente clases de core; se usa, por ejemplo, para cargar class.restazo.php
	 *
	 * @param string $sClass Clase a cargar
	 * @return boolean
	 */
	public static function autoload($sClass){
		$bLeido = false;
		$sClass = strtolower($sClass); //por ahora los nombres de las clases van en minusculas

		//es una clase core
		$sClaseCore = D_BASE_DIR . D_DIR_CORE  . 'class.' . $sClass . '.php';
		if(is_readable($sClaseCore)){
			$bLeido = include_once $sClaseCore;
		}

		/*if(!$bLeido){
			//clase o ruta desconocida
			throw new ErrorException('Error fatal: No se puede cargar la clase [' . $sClass . ']');
		}*/

		return $bLeido;
	}

	/**
	 * Crea las constantes BASE_DIR, BASE_URL y D_BASE_URL_FQDN
	 *
	 * D_BASE_URL_FQDN es igual que D_BASE_URL pero con protocolo, servidor y demas
	 */
	private function baseDir(){
		if(!defined('D_BASE_DIR')){
			define('D_BASE_DIR', str_replace('\\', '/', realpath(dirname(__FILE__).'/..')).'/');
		}

		if(!defined('D_BASE_URL')){
			$aBaseDir = explode('/', trim(D_BASE_DIR, '/'));
			$sCadBase = array_pop($aBaseDir);
			$sRequestUri = $_SERVER['REQUEST_URI'];

			$sCadBase = strpos($sRequestUri, $sCadBase) === false ? $sRequestUri : ltrim(substr($sRequestUri, strpos($sRequestUri, $sCadBase) + strlen($sCadBase)), '/');
			//$sCadBase = ($sCadBase === false) ? array() : explode('/', trim($sCadBase, '/'));
			//define('D_BASE_URL', str_repeat('../', count($sCadBase)));
			$iNumSaltos = substr_count($sCadBase, '/');
			define('D_BASE_URL', str_repeat('../', $iNumSaltos));

			if(!defined('D_BASE_URL_FQDN')){
				$sPhpSelf = trim(dirname($_SERVER['PHP_SELF']), '/');
				//$sUrl = $_SERVER['REQUEST_SCHEME'] . '://' . $_SERVER['SERVER_NAME'] . $_SERVER['SCRIPT_NAME'] . '/imgs/captcha.php';
				$sUrl = $_SERVER['REQUEST_SCHEME'] . '://' . $_SERVER['SERVER_NAME'] . (empty($sPhpSelf) ? '' : '/' . $sPhpSelf) . '/';
				define('D_BASE_URL_FQDN', $sUrl);
			}
		}
	}

	/**
	 * Verifica si existe el modulo indicado en la URL,
	 * si no existe busca el modulo de error,
	 * si tampoco existe avisa con un mensaje
	 *
	 * @todo verificar orden de busqueda y operaciones de lectura de disco, quizas pueda hacerse todo con el array_map de glob, comprobando antes que modulo se ha pedido (y quizas el de error tambien)
	 * @return void
	 */
	private function buscaModulo(){
		//seleccion de modulo, portada por defecto
		$this->sModulo = empty($_GET) ? D_MODULO_INICIO : key($_GET); //nombre del modulo

		//acepta modulo si encuentra el mismo nombre entre controladores o vistas
		$this->bVista = in_array($this->sModulo . '.php', array_map('basename', glob(D_BASE_DIR . D_DIR_VISTA . '*.php'))); //se pide una vista
		$this->bControlador = in_array($this->sModulo . '.php', array_map('basename', glob(D_BASE_DIR . D_DIR_CONTROL . '*.php'))); //se pide un controlador

		//portada o seccion desconocida
		if(!$this->bVista && !$this->bControlador){
			if(empty($_SERVER['QUERY_STRING']) || isset($_GET[D_MODULO_INICIO])){
				$this->sModulo = D_MODULO_INICIO;
			}
			else{
				$this->sModulo = D_MODULO_ERROR; //TODO enviar cabecera con codigo de error?
			}
		}

		//si existe el controlador se prioriza su carga
		if(is_readable(D_BASE_DIR . D_DIR_CONTROL . $this->sModulo . '.php')){
			$this->bControlador = true;
		}
		elseif(is_readable(D_BASE_DIR . D_DIR_VISTA . $this->sModulo . '.php')){ //si no se sabe lo que existe se intenta en el directorio de vistas
			$this->bVista = true;
		}
		else{
			$this->bVista = $this->bControlador = null;
			throw new ErrorException('Error fatal: No se puede cargar el m&oacute;dulo [' . $this->sModulo . '] en [' . $sRutaCompleta . ']');
		}
	}

	/**
	 * Devuelve la posicion que ocupa $sBuscarClase entre los antecesores de $sClase; ignora el nombre de la propia clase $sClase
	 *
	 * @param string $sClase Clase hija en la que buscar antecesores
	 * @param string $sBuscarClase Clase antecesora a buscar en la jerarquia de clase hija
	 * @return integer 0 si no se encuentra, 1 o mayor siendo la posicion donde se encuentre $sBuscarClase
	 */
	private function antecesores($sClase, $sBuscarClase){
		$iPos = 0;
		$sClaseAux = $sClase;

		for($aClases[] = $sClaseAux; $sClaseAux = get_parent_class($sClaseAux); $aClases[] = $sClaseAux);

		array_shift($aClases); //quita la clase en la que se ha buscado la jerarquia
		if(count($aClases)){ //si la clase tenia herencia se busca la coincidencia
			$aClases = array_reverse($aClases);
			$iAux = array_search($sBuscarClase, $aClases);
			if($iAux !== false){
				$iPos = $iAux + 1;
			}
		}

		//return $aClases;
		return $iPos;
	}

	/**
	 * Carga el controlador relacionado con el modulo, deja una instancia del controlador en $this->oControlazo
	 *
	 * Si el valor del parametro modulo es un metodo intenta cargarse y ejecutarse
	 *
	 * @return void
	 */
	private function cargaControlador(){
		//clases base de controlador y modelo
		require D_BASE_DIR . D_DIR_CORE . 'class.controlazo.php';
		require D_BASE_DIR . D_DIR_CORE . 'class.modelazo.php';

		$sRutaControlador = D_BASE_DIR . D_DIR_CONTROL . $this->sModulo . '.php';

		//intenta cargar el controlador (puede contener tambien el modelo)
		if(is_readable($sRutaControlador)){
			include $sRutaControlador;
		}

		//carga el controlador si existe
		$sControlador = ucfirst(strtolower($this->sModulo));

		if($this->antecesores($sControlador, 'Restazo')){ //controlador Restazo
			$this->oRestazo = new $sControlador();
		}
		elseif(class_exists($sControlador) && $this->antecesores($sControlador, 'Controlazo')){ //controlador normal
			//si se ha pedido un metodo concreto (valor del parametro modulo) se carga y se le pasan el resto de parametros
			//TODO eliminar nombres de metodo no permitidos? (construct, destruct, ...)
			$sMethod = D_METODO_INICIO; //se intenta cargar el metodo por defecto
			if(isset($_GET[$this->sModulo]) && $_GET[$this->sModulo] != ''){
				$sMethod = $_GET[$this->sModulo];
			}

			if(method_exists($sControlador, $sMethod)){
				$this->oControlazo = new $sControlador();

				$aParametros = $_GET;
				array_shift($aParametros); //quita el primer parametro, que es el controlador y el metodo (dominio.tld?controlador=metodo)
				$mReturn = call_user_func_array(array($this->oControlazo, $sMethod), $aParametros); //TODO hacer algo con el return?
			}
			else{ //TODO no se ha encontrado el metodo (ni el metodo por defecto)
				$this->sModulo = D_MODULO_ERROR;

				$sRutaControlador = D_BASE_DIR . D_DIR_CONTROL . $this->sModulo . '.php';

				if(is_readable($sRutaControlador)){
					include_once $sRutaControlador; //FIXME en caso de error, ya se ha cargado antes y aqui produce error sin el _once

					$this->oControlazo = new $this->sModulo();
				}
				else{
					throw new ErrorException('Error fatal: No se puede cargar el m&oacute;dulo ['.$this->sModulo.'] en [' . $sRutaControlador . ']');
				}
			}
		}
		else{
			if(is_readable(D_BASE_DIR . D_DIR_VISTA . $this->sModulo . '.php')){ //intenta cargar la vista con el mismo nombre
				$this->oControlazo = new Controlazo(D_BASE_DIR . D_DIR_VISTA . $this->sModulo . '.php');
			}
			elseif(class_exists('Controlazo')){ //si no existe el controlador del modulo pedido ni la vista se instancia el controlador base
				$this->oControlazo = new Controlazo();
				//TODO llamar a un metodo pedido en la URL?
			}

			$this->oControlazo->pinta($this->aFicheros['plantilla']); //pinta automaticamente vistas (sin controlador asociado) con la plantilla por defecto
		}
	}

	/**
	 * Guarda la IP del cliente actual
	 *
	 * @return void
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
	 * @return void
	 */
	private function pintaPagina(){
		if(!headers_sent($file, $line)){
			header('Content-Type: text/html; charset=UTF-8');
		}
		elseif(D_DEBUG){
			echo 'Headers ya enviados en ['.$file.'] en linea ['.$line . ']';
		}

		/*construye pagina*/
		$this->buscaModulo();
		$this->cargaControlador();

		if(D_DEBUG){
			echo('Uso de traducciones:');
			print_r(l10n_sel()->getTraduccionUso(-1));
			//echo('URL:');
			//print_r($_GET);
			//print_r($_SERVER);
		}
	}

	/**
	 * Pone la aplicacion en modo debug o no
	 *
	 * En modo debug se muestran errores
	 *
	 * @return void
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
	 *
	 * @return void
	 */
	private function tempDir(){
		$sTempDir = null;

		if(function_exists('sys_get_temp_dir')){
			$sTempDir = realpath(sys_get_temp_dir());
		}
		else{
			if($temp = getenv('TMP')){
				return $temp;
			}
			if($temp = getenv('TEMP')){
				return $temp;
			}
			if($temp = getenv('TMPDIR')){
				return $temp;
			}
			$temp = tempnam(dirname(__FILE__), '');
			if(file_exists($temp)){
				unlink($temp);
				$sTempDir = dirname($temp);
			}
		}

		define('D_TEMP_PATH', $sTempDir);
	}

}
