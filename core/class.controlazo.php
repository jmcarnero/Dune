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
 * @version 1
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
	 * @see Controlazo::pinta($plantilla)
	 */
	protected $sContenidos = '';

	/**
	 * Constructor
	 *
	 * @throws ErrorException
	 * @param string $sFile Ruta del modulo a cargar
	 */
	public function __construct($sFile = false){
		if($sFile === false){
			$sFile = __FILE__;
		}

		$this->rutaModulo($sFile); //TODO depende de que el usuario haga la llamada correcta a este constructor
		if(empty($this->sModulo)){
			throw new ErrorException('El constructor de su controlador debe incluir: "parent::__construct(__FILE__);"');
		}

		if(is_readable(D_BASE_DIR . D_DIR_LIBS . 'class.l10n.inc')){
			$sLang = empty($_SESSION['D_idioma']) ? (defined('D_IDIOMA_DEFECTO') ? D_IDIOMA_DEFECTO : false) : $_SESSION['D_idioma'];
			//internacionalizacion y localizacion de la pagina
			include_once D_BASE_DIR . D_DIR_LIBS . 'class.l10n.inc';
			l10n_sel($sLang, false, 'csv', D_BASE_DIR . 'l10n');
			$sLocale = _locale($sLang);
			//var_dump($sLocale, date_default_timezone_get());
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
	 * si $sLib = 'nombre_clase'
	 * el fichero debe ser "class.nombre_clase.inc"
	 * y la clase debe llamarse 'Nombre_clase'
	 *
	 * @todo no permitir la carga de redatazo (ya lo hace el modelo)
	 * @todo restringir nombres en uso de Dune para valores de $sLib
	 * @param string $sLib Nombre de la libreria a cargar; corresponde a nombres de clases en el directorio de librerias
	 * @param array $aParametros Array de parametros a pasar al nuevo constructor
	 * @param string $sNombre Nombre con que se instanciara la clase para ser usada en siguientes llamadas, por defecto el mismo que $sLib
	 * @return boolean True si se ha cargado correctamente, false si no
	 */
	protected function carga($sLib = null, $aParametros = array(), $sNombre = null){
		if(empty($sLib)){
			return false; //no se ha pedido nada para cargar
		}
		if(empty($sNombre)){
			$sNombre = $sLib;
		}

		if(is_readable(D_BASE_DIR . D_DIR_LIBS . 'class.' . $sLib . '.inc')){
			include_once(D_BASE_DIR . D_DIR_LIBS . 'class.' . $sLib . '.inc');
		}

		$sClase = ucfirst($sLib); //$sLib == 'nombre_clase', el fichero es "class.nombre_clase.inc", la clase debe ser "Nombre_clase"

		//carga la clase si existe
		if(class_exists($sClase) && !isset($this->$sClase)){
			if(empty($aParametros))
				$this->$sNombre = new $sClase();
			else
				$this->$sNombre = new $sClase($aParametros); //TODO de momento solo acepta un array de parametros o nada

			return true;
		}

		return false;
	}

	/**
	 * Carga el modelo de datos
	 *
	 * Por defecto carga el modelo de la propia pagina, que se asignara a $this->oModelo
	 * Si $sModelo = 'nombre_modelo'
	 * el fichero debe ser "nombre_modelo.php"
	 * y la clase debe llamarse 'nombre_modelo'
	 *
	 * @todo restringir nombres en uso de Dune para valores de $sModelo
	 * @param string $sFichero Nombre del modelo a cargar; corresponde a nombres de ficheros (sin extension) en el directorio de modelos
	 * @param string $sNombre Nombre con que se instanciara el modelo para ser usado en siguientes llamadas, por defecto el mismo que $sModelo
	 * @return boolean True si se ha cargado correctamente, false si no
	 */
	protected function cargaModelo($sFichero = null, $sNombre = null){
		if(empty($sFichero)){
			$sFichero = $this->sModulo;

			if(empty($sNombre))
				$sNombre = 'oModelo';
		}
		elseif(empty($sNombre)){
			$sNombre = $sFichero;
		}

		//ruta/nombre del fichero de modelo
		$sModelo = ucfirst($sFichero) . D_SUFIJO_MODELO;
		$sRuta = strtolower($sFichero) . '.php';

		//intenta cargar el modelo
		if(is_readable(D_BASE_DIR . D_DIR_MODEL . $sRuta)){
			include_once(D_BASE_DIR . D_DIR_MODEL . $sRuta);

			if(class_exists($sModelo)){
				$this->$sNombre = new $sModelo();
				return true;
			}
		}

		return false; //no se ha cargado el modelo o ya estaba cargado //TODO mensaje de error si ha habido problemas en la carga
	}

	/**
	 * Devuelve, si existe, la clave $clave en los super globales GET, POST, REQUEST o SESSION
	 *
	 * @since 2015-02-28
	 * @param string $sClave Clave a buscar
	 * @param string $sGlobal SuperGlobal en la que buscar ('get', 'post', 'request', 'session')
	 * @return string Valor encontrado, null si no encontrado
	 */
	protected function dato($sClave, $sGlobal = null){
		$sRet = null;
		$sGlobal = empty($sGlobal) ? null : '_' . strtoupper($sGlobal);

		$aSGlobal = array('_POST', '_GET', '_SESSION', '_REQUEST'); //lista de super globales en los que buscar $clave, devuelve el primero que corresponda (en el orden de este array), a no ser que se pida $sglobal

		if(in_array($sGlobal, $aSGlobal)){
			if(!empty($sGlobal) && isset($GLOBALS["$sGlobal"][$sClave])){
				$sRet = $GLOBALS["$sGlobal"][$sClave];
			}
		}
		else{
			foreach($aSGlobal as $global){
				if(isset($GLOBALS["$global"][$sClave])){
					$sRet = $GLOBALS["$global"][$sClave];
					break;
				}
			}
		}

		return $sRet;
	}

	/**
	 * Devuelve, si existe, la clave $clave en el super globales GET
	 *
	 * @see Controlazo::dato($sClave, $sGlobal)
	 * @param string $sClave Clave a buscar
	 * @return string Valor encontrado, null si no encontrado
	 */
	protected function get($sClave){
		return $this->dato($sClave, 'get');
	}

	/**
	 * Compone la pagina y la envia al navegador
	 * Si no se quiere plantilla (como cuando se ha de cargar la pagina por AJAX) sobreescribir este metodo en el controlador heredado sin parametro plantilla (o como sea necesario)
	 *
	 * @param string $sPlantilla nombre de la plantilla a cargar, en el directorio D_DIR_TPL, sin extension (extension .tpl)
	 * @param string $sVista Vista a pintar, en el directorio D_DIR_VISTA, sin extension; por defecto se carga la que tenga el mismo nombre de controlador
	 * @return void
	 */
	public function pinta($sPlantilla = false, $sVista = false){
		if(empty($sVista)){
			$sVista = $this->sModulo;
		}

		if(!empty($this->aDatos)){
			foreach($this->aDatos as $clave => $valor){
				$$clave = $valor; //cada clave del array de datos se podra usar como una variable directa en la plantilla, igual que los contenidos obtenidos del modulo (abajo, $sContenidos)
			}
		}

		$sRutaVista = D_BASE_DIR . D_DIR_VISTA . $sVista . '.php';

		if(!is_readable($sRutaVista)){
			throw new ErrorException('No se puede leer la vista [' . $sRutaVista . ']');
		}

		ob_start();
		include $sRutaVista; //carga del modulo, debe tener el mismo nombre de esta clase (en minusculas)

		$this->sContenidos = ob_get_contents(); //contenidos del modulo a mostrar procesados
		ob_end_clean();

		if(!empty($sPlantilla)){
			$sRutaPlantilla = D_BASE_DIR . D_DIR_TPL . $sPlantilla . '.tpl';

			if(!is_readable($sRutaPlantilla)){
				throw new ErrorException('No se puede leer la plantilla [' . $sRutaPlantilla . ']');
			}

			$sContenidos = $this->sContenidos;
			require $sRutaPlantilla;
		}
		elseif(!empty($sVista)){ //si hay vista pero no plantilla se pinta //TODO debiera no pintarse nada si no se pasa plantilla?
			echo $this->sContenidos;
		}
	}

	/**
	 * Devuelve, si existe, la clave $clave en el super globales POST
	 *
	 * @see Controlazo::dato($sClave, $sGlobal)
	 * @param string $sClave Clave a buscar
	 * @return string Valor encontrado, null si no encontrado
	 */
	protected function post($sClave){
		return $this->dato($sClave, 'post');
	}

	/**
	 * Separa ruta y nombre del modulo a partir de la constante __FILE__
	 *
	 * @todo no contempla rutas windows
	 * @param string $sFile Ruta del fichero
	 * @return void
	 */
	private function rutaModulo($sFile){
		$this->sModulo = substr(basename($sFile), 0, strpos(basename($sFile), '.'));

		//puede instanciarse este controlador base (que no tiene sufijo); por ejemplo para llamar al modulo de error
		if(empty($this->sModulo) && stripos(basename($sFile), __CLASS__) > 0) $this->sModulo = __CLASS__;
	}

	/**
	 * Traduccion de textos
	 *
	 * @param string $sCadena Cadena a traducir
	 * @return void
	 */
	protected function trad($sCadena = ''){
		if(function_exists('_tradR')){
			$sCadena = _tradR($sCadena);
		}

		return $sCadena;
	}

}
