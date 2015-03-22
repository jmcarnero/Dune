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

/**
 * Clase controlador generico
 *
 * @author José M. Carnero
 * @since 2014-11-17
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Controlazo {

	protected $aSesion = array();

	protected $oModelo; //modelo correspondiente a este controlador ("Modulo_model")
	private $sModulo = ''; //nombre del modulo que se ha cargado, lo envia el constructor hijo

	/**
	 * Este array contiene todos los datos que se podran usar como variables en los modulos/plantillas,
	 * ej.: $this->aDatos['variable'] podra usarse como $variable en el modulo/plantilla
	 *
	 * @var array
	 */
	protected $aDatos = array(
			'aMod' => false, //nombre y titulo visible del modulo
			'sMetas' => false, //tags meta
			'sTitle' => false, //titulo que tendra la pagina generada por el modulo
			'sCss' => false, //tag style o link a insertar en el head de la pagina
			'sJs' => false, //tag script a insertar en el head de la pagina (javascript)
		);

	/**
	 * Contenidos del modulo a mostrar despues de ser procesados y listo para ser integrados en la plantilla o enviados al navegador
	 *
	 * @var string
	 * @see Controlazo::pintaPagina($plantilla)
	 */
	protected $sContenidos = '';

	function __construct($file = false){
		if($file === false) $file = __FILE__;

		$this->rutaModulo($file); //TODO depende de que el usuario haga la llamada correcta a este constructor
		if(empty($this->sModulo)){
			throw new ErrorException('El constructor de su controlador debe incluir: "parent::__construct(__FILE__);"');
		}

		if(is_readable(D_BASE_DIR . D_DIR_LIBS . 'class.l10n.inc')){
			$sLang = empty($_SESSION['D_idioma']) ? (defined('D_IDIOMA_DEFECTO') ? D_IDIOMA_DEFECTO : false) : $_SESSION['D_idioma'];
			//internacionalizacion y localizacion de la pagina
			include_once (D_BASE_DIR . D_DIR_LIBS . 'class.l10n.inc');
			l10n_sel($sLang, false, 'csv', D_BASE_DIR . 'l10n');
			_locale($sLang);
			//_trad('clave_test');
		}

		$this->cargaModelo();

		$this->aDatos['aMod'][$this->sModulo] = array('title' => $this->trad($this->sModulo));

		$this->aDatos['sMetas'] = '';
		$this->aDatos['sTitle'] = $this->aDatos['aMod'][$this->sModulo]['title'];
		$this->aDatos['sCss'] .= '';
		$this->aDatos['sJs'] .= '';

		//mensajes devueltos por el procesado de formularios y otros eventos
		$sMensajes = false;
		if(!empty($_SESSION['D_mensajes'])){
			$sMensajes = $_SESSION['D_mensajes']; //TODO borrar o conservar este mensaje?
		}
	}

	/**
	 * Carga la libreria que se pida
	 * si $lib = 'nombre_clase'
	 * el fichero debe ser "class.nombre_clase.php"
	 * y la clase debe llamarse 'nombre_clase'
	 *
	 * @todo no permitir la carga de redatazo (ya lo hace el modelo)
	 * @param string $lib Nombre de la libreria a cargar; corresponde a nombres de clases en el directorio de librerias
	 * @param array $parametros Array de parametros a pasar al nuevo constructor
	 * @param string $nombre Nombre con que se instanciara la clase para ser usada en siguientes llamadas, por defecto el mismo que $lib
	 * @return boolean True si se ha cargado correctamente, false si no
	 */
	protected function carga($lib = false, $parametros = array(), $nombre = null){
		if(empty($lib)){
			return false; //no se ha pedido nada para cargar
		}
		if(empty($nombre))
			$nombre = $lib;

		if(is_readable(D_BASE_DIR . D_DIR_LIBS . 'class.' . $lib . '.inc')){
			include_once (D_BASE_DIR . D_DIR_LIBS . 'class.' . $lib . '.inc');
		}

		//carga la clase si existe
		if(class_exists($lib) && !isset($this->$lib)){
			if(empty($parametros))
				$this->$nombre = new $lib();
			else
				$this->$nombre = new $lib($parametros); //TODO de momento solo acepta un array de parametros o nada

			return true;
		}

		return false;
	}

	//intenta instanciar el modelo de datos
	protected function cargaModelo(){
		//ruta/nombre del fichero de modelo
		$sModelo = ucfirst($this->sModulo) . D_SUFIJO_MODELO;
		$sRuta = strtolower($this->sModulo) . '.php';

		//intenta cargar el modelo
		if(empty($this->oModelo) && is_readable(D_BASE_DIR . D_DIR_MODEL . $sRuta)){
			include (D_BASE_DIR . D_DIR_MODEL.$sRuta);
		}

		if(class_exists($sModelo)){
			$this->oModelo = new $sModelo();
			return true;
		}

		return false; //no se ha cargado el modelo o ya estaba cargado //TODO mensaje de error si ha habido problemas en la carga
	}

	/**
	 * Compone la pagina y la envia al navegador
	 * Si no se quiere plantilla (como cuando se ha de cargar la pagina por AJAX) sobreescribir este metodo en el controlador heredado sin parametro plantilla (o como sea necesario)
	 *
	 * @param string $plantilla Ruta y nombre de la plantilla a cargar
	 */
	public function pintaPagina($plantilla = false){
		if(!empty($this->aDatos)){
			foreach($this->aDatos as $clave => $valor){
				$$clave = $valor; //cada clave del array de datos se podra usar como una variable directa en la plantilla, igual que los contenidos obtenidos del modulo (abajo, $sContenidos)
			}
		}

		ob_start();
		if(is_readable(D_BASE_DIR . D_DIR_VISTA . $this->sModulo . '.php')){
			include (D_BASE_DIR . D_DIR_VISTA . $this->sModulo . '.php'); //carga del modulo, debe tener el mismo nombre de esta clase (en minusculas)
		}
		$this->sContenidos = ob_get_contents(); //contenidos del modulo a mostrar procesados
		ob_end_clean();

		if($plantilla && is_readable($plantilla)){
			$sContenidos = $this->sContenidos;
			require $plantilla;
		}
	}

	/**
	 * Devuelve, si existe, la clave $clave en los super globales GET o POST
	 *
	 * @since 2015-02-28
	 * @param string $clave Clave a buscar
	 * @param string $sglobal SuperGlobal en la que buscar ('post', 'get')
	 * @return string Valor encontrado, null si no encontrado
	 */
	protected function post($clave, $sglobal = null){
		$sRet = null;
		$sglobal = empty($sglobal) ? null : '_' . strtoupper($sglobal);

		$aSGlobal = array('_POST', '_GET'); //lista de super globales en los que buscar $clave, devuelve el primero que corresponda (en el orden de este array), a no ser que se pida $sglobal

		if(in_array($sglobal, $aSGlobal)){
			if(!empty($sglobal) && isset($GLOBALS["$global"][$clave]))
				$sRet = $GLOBALS["$global"][$clave];
		}
		else{
			foreach($aSGlobal as $global){
				if(isset($GLOBALS["$global"][$clave])){
					$sRet = $GLOBALS["$global"][$clave];
					break;
				}
			}
		}

		return $sRet;
	}

	//separa ruta y nombre del modulo a partir de la constante __FILE__
	//TODO no contempla rutas windows
	private function rutaModulo($file){
		$this->sModulo = substr(basename($file), 0, strpos(basename($file), '.'));

		//puede instanciarse este controlador base (que no tiene sufijo); por ejemplo para llamar al modulo de error
		if(empty($this->sModulo) && stripos(basename($file), __CLASS__) > 0) $this->sModulo = __CLASS__;
	}

	 //traduccion de textos
	protected function trad($cadena = ''){
		if(function_exists('_tradR')){
			$cadena = _tradR($cadena);
		}

		return $cadena;
	}

}
