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
 * - modulo -> el modulo a cargar; si se omite se cargara el modulo por defecto (portada), si no se encuentra se cargara el modulo de error (si no se encuentra se envia un mensaje de aviso)
 * - metodo -> metodo a cargar del modulo pedido; se puede omitir, si no existe se ignora
 * - parametro=valor -> cualquier numero de parametros con o sin valor, en el formato ordinario de una URL (separados por &); de momento se recogen por post, no se pasan como parametros al metodo llamado
 *
 * @author José M. Carnero
 * @since 2014-11-23
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Dune {

	protected $sContenidos = false; //contenidos cargados de la vista/controlador/modelo
	protected $oControlazo = false; //instancia de la clase controlador

	/**
	 * Nombres de ficheros de configuracion, plantilla base, etc.
	 * - config: fichero de configuracion
	 * - plantilla: fichero base de plantilla
	 * - sesion: controlador de sesion //TODO de momento no integrado como objeto
	 *
	 * @var array
	 */
	protected $aFicheros = array('config' => 'config.php', 'plantilla' => 'dune.tpl', 'sesion' => 'sesion.php');

	private $sModulo = 'error'; //nombre del modulo a cargar

	function __construct(){
		Dune::baseDir();

		//includes comunes
		require(D_BASE_DIR.D_DIR_LIBS.$this->aFicheros['config']); //parametros basicos de la aplicacion
		$this->setDebug();
		require(D_BASE_DIR.D_DIR_LIBS.$this->aFicheros['sesion']); //variables e inicio de sesion

		//gestion de errores
		if(D_DEBUG){
			include(D_BASE_DIR.D_DIR_CORE.'class.errorHandler.inc');
			//logFile('errors.err');
		}

		$this->ipCliente();
		$this->tempDir();

		define('LOGOUT', D_BASE_URL.'?logout');

		/* final */
		$this->pintaPagina();
	}

	/**
	 * Crea las constantes BASE_DIR y BASE_URL,
	 * por esto la clase init debe estar en el directorio raiz
	 *
	 * @param string $ret Devuelve la URL (si se pasa 'url') o directorio base (si se pasa 'dir'); si no devuelve null
	 * @return string
	 */
	public static function baseDir($ret = null){
		if(!defined('D_BASE_DIR')) define('D_BASE_DIR', str_replace('\\', '/', realpath(dirname(__FILE__).'/..')).'/');
		if(!defined('D_BASE_URL')){
			$cadBase = array_pop(explode('/', trim(D_BASE_DIR, '/')));
			$phpSelf = trim(dirname($_SERVER['PHP_SELF']), '/');
			$cadBase = substr($phpSelf, strpos($phpSelf, $cadBase) + strlen($cadBase));
			$cadBase = $cadBase === false?array():explode('/', trim($cadBase, '/'));
			define('D_BASE_URL', str_repeat('../', count($cadBase)));
		}

		switch($ret){
			case 'dir':
				return D_BASE_DIR;
				break;
			case 'url':
				return D_BASE_URL;
				break;
			default:
				return null;
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

		if(in_array($this->sModulo.'.php', array_map('basename', glob(D_BASE_DIR.D_DIR_VISTA.'*.php')))){ //TODO deben priorizarse vistas o controladores?, de momento vistas
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
				$this->sModulo = 'error'; //TODO enviar cabecera con codigo de error?
			}
		}

		if(!is_readable(D_BASE_DIR.D_DIR_VISTA.$this->sModulo.'.php')){
			throw new ErrorException('Error fatal: No se puede cargar el m&oacute;dulo ['.$this->sModulo.']');
		}
	}

	/**
	 * Carga el controlador relacionado con el modulo,
	 * deja una instancia del controlador en $this->oControlazo
	 */
	private function cargaControlador(){
		//clases base de controlador y modelo
		require(D_BASE_DIR.D_DIR_CORE.'class.controlazo.php');
		require(D_BASE_DIR.D_DIR_CORE.'class.modelazo.php');

		//intenta cargar el controlador (puede contener tambien el modelo)
		if(is_readable(D_BASE_DIR.D_DIR_CONTROL.$this->sModulo.'.php')){
			include(D_BASE_DIR.D_DIR_CONTROL.$this->sModulo.'.php');
		}

		//carga el controlador si existe
		$aux = ucfirst(strtolower($this->sModulo));
		if(class_exists($aux)){
			$this->oControlazo = new $aux();
			//TODO llamar a un metodo pedido en la URL?
		}
		else{
			//intenta cargar la vista con el mismo nombre
			if(is_readable(D_BASE_DIR.D_DIR_VISTA.$this->sModulo.'.php')){
				$this->oControlazo = new Controlazo(D_BASE_DIR.D_DIR_VISTA.$this->sModulo.'.php');
			}
			elseif(class_exists('Controlazo')){ //si no existe el controlador del modulo pedido ni la vista se instancia el controlador base
				$this->oControlazo = new Controlazo();
				//TODO llamar a un metodo pedido en la URL?
			}
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
			echo 'Headers ya enviados en ['.$file.'] en linea ['.$line . ']';
		}

		/*construye pagina*/
		$this->buscaModulo();
		$this->cargaControlador();

		$this->oControlazo->pintaPagina(D_BASE_DIR.D_DIR_TPL.$this->aFicheros['plantilla']);

		/*if(D_DEBUG){
			echo('uso de traducciones:');print_r(l10n_sel()->getTraduccionUso(-1));
		}*/
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

}
