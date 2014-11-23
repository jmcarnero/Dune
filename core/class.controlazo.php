<?php
//namespace Sargazos.net;

/**
 * Clase controlador generico
 *
 * @since 2014-11-17
 * @author jm_carnero@sargazos.net
 * @abstract
 */
abstract class Controlazo {

	protected $oModel;

	protected $aMod;
	protected $sMetas;
	protected $sTitle;
	protected $sCss;
	protected $sJs;

	//datos listos para usar en el pintado de la plantilla
	protected $aDatos = array();

	function __construct(){
		/*$this->cargaModelo(__CLASS__);*/

		$this->aMod[__CLASS__] = array('title' => $this->trad(__CLASS__));

		$this->sMetas = '';
		$this->sTitle = $this->aMod[__CLASS__]['title'];
		$this->sCss .= '';
		$this->sJs .= '';
	}

	//intenta instanciar el modelo de datos
	protected function cargaModelo($clase){
		$clase = $clase.'_model';
		if(class_exists($clase)){
			$this->oModel = new $clase();
		}
	}

	//devuelve los datos que se usaran en el pintado de la plantilla
	public function getDatos(){
		return $aDatos;
	}

	private function trad($cadena){
		if(function_exists('_tradR')){
			$cadena = _tradR($cadena);
		}

		return $cadena;
	}

}
