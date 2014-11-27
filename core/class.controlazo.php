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

	/**
	 * Rutas a los directorios del framework; se pasan desde dune.php
	 * - libs: librerias
	 * - mods: modulos, directorio para los ficheros de vistas, controladores y modelos de cada modulo
	 * - tpl: plantillas, ficheros con extension ".tpl"
	 *
	 * @var array
	 * @see Controlazo::setDirectorios($aDirectorios)
	 */
	protected $aDirectorios = array('libs' => 'incs/', 'mods' => 'mods/', 'tpl' => 'tpl/');

	protected $oModelo; //modelo correspondiente a este controlador ("Modulo_model")
	private $aModulo = array('path' => '', 'mod' => ''); //path y nombre del modulo que se ha cargado, lo envia el constructor hijo

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

	function __construct($file = false){
		if($file === false) $file = __FILE__;

		$this->rutaModulo($file); //TODO depende de que el usuario haga la llamada correcta a este constructor
		if(empty($this->aModulo['mod'])){
			throw new ErrorException('El constructor de su controlador debe incluir como primera instruccion: "parent::__construct(__FILE__);"');
		}
		/*//internacionalizacion y localizacion de la pagina
		//LENGUAJE_DEFECTO
		include_once(BASE_DIR.$this->aDirectorios['libs'].'class.l10n.inc');
		l10n_sel();
		_locale();
		//_trad('clave_test');*/

		$this->cargaModelo();

		$this->aDatos['aMod'][$this->aModulo['mod']] = array('title' => $this->trad($this->aModulo['mod']));

		$this->aDatos['sMetas'] = '';
		$this->aDatos['sTitle'] = $this->aDatos['aMod'][$this->aModulo['mod']]['title'];
		$this->aDatos['sCss'] .= '';
		$this->aDatos['sJs'] .= '';

		//mensajes devueltos por el procesado de formularios y otros eventos
		$sMensajes = false;
		if(!empty($_SESSION['mensajes'])){
			$sMensajes = $_SESSION['mensajes']; //TODO borrar o conservar este mensaje?
		}
	}

	/**
	 * Carga la libreria que se pida
	 * si $lib = 'nombre_clase'
	 * el fichero debe ser "class.nombre_clase.php"
	 * y la clase debe llamarse 'nombre_clase'
	 *
	 * @todo no permitir la carga de sqlazo (ya lo hace el modelo)
	 * @param string $inc Nombre de la libreria a cargar; corresponde a nombres de clases en el directorio de librerias
	 * @return boolean True si se ha cargado correctamente, false si no
	 */
	protected function carga($lib = false){
		if(empty($lib)){
			return false; //no se ha pedido nada para cargar
		}

		if(is_readable(D_BASE_DIR.$this->aDirectorios['libs'].'class.'.$lib.'.inc')){
			include_once(D_BASE_DIR.$this->aDirectorios['libs'].'class.'.$lib.'.inc');
		}

		//carga el modelo si existe
		if(class_exists($lib) && !isset($this->$lib)){
			$this->$lib = new $lib();
			return true;
		}

		return false;
	}

	//intenta instanciar el modelo de datos
	public function cargaModelo(){
		//ruta/nombre del fichero de modelo
		$sModelo = ucfirst($this->aModulo['mod']).D_SUFIJO_MODELO;
		$sRuta = strtolower($this->aModulo['mod']).D_SUFIJO_MODELO.'.php';

		//intenta cargar el modelo
		if(empty($this->oModelo) && is_readable($this->aModulo['path'].$sRuta)){
			include($this->aModulo['path'].$sRuta);
		}

		if(class_exists($sModelo)){
			$this->oModelo = new $sModelo();
			$this->oModelo->setDirectorioLibs($this->aDirectorios['libs']);
			//return true;
		}

		return false; //no se ha cargado el modelo o ya estaba cargado //TODO mensaje de error si ha habido problemas en la carga
	}

	/**
	 * Compone la pagina y la envia al navegador
	 *
	 * @param string $plantilla Ruta y nombre de la plantilla a cargar
	 */
	public function pintaPagina($plantilla = false){
		ob_start();

		if(is_readable($this->aModulo['path'].$this->aModulo['mod'].'.php')){
			include($this->aModulo['path'].$this->aModulo['mod'].'.php'); //carga del modulo, debe tener el mismo nombre de esta clase (en minusculas) y estar en el mismo directorio
		}

		$sContenidos = ob_get_contents(); //contenidos del modulo a mostrar procesados

		ob_end_clean();

		if(!empty($this->aDatos)){
			foreach($this->aDatos as $clave => $valor){
				$$clave = $valor; //cada clave del array de datos se podra usar como una variable directa en la plantilla, igual que los contenidos obtenidos del modulo (abajo, $sContenidos)
			}
		}

		require($plantilla);
	}

	//separa ruta y nombre del modulo a partir de la constante __FILE__
	//TODO no contempla rutas windows
	private function rutaModulo($file){
		$this->aModulo['path'] = dirname($file) . '/';

		$this->aModulo['mod'] = substr(basename($file), 0, strpos(basename($file), D_SUFIJO_CONTROLADOR));

		//puede instanciarse este controlador base (que no tiene sufijo); por ejemplo para llamar al modulo de error
		if(empty($this->aModulo['mod']) && stripos(basename($file), __CLASS__) > 0) $this->aModulo['mod'] = __CLASS__;
	}

	/**
	 * Asigna los directorios de la aplicacion, llamada desde dune.php
	 */
	public function setDirectorios($aDirectorios){
		if(!empty($aDirectorios['libs'])) $this->aDirectorios['libs'] = $aDirectorios['libs'];
		if(!empty($aDirectorios['mods'])) $this->aDirectorios['mods'] = $aDirectorios['mods'];
		if(!empty($aDirectorios['tpl'])) $this->aDirectorios['tpl'] = $aDirectorios['tpl'];
	}

	 //traduccion de textos
	protected function trad($cadena){
		if(function_exists('_tradR')){
			$cadena = _tradR($cadena);
		}

		return $cadena;
	}

}
