<?php
//namespace Dune;

/**
 * Clase controlador generico
 *
 * @since 2014-11-17
 * @author jm_carnero@sargazos.net
 * @abstract
 */
abstract class Controlazo {

	protected $oModel;

	/**
	 *
	 * @var array
	 */
	protected $aDatos = array(
			'aMod' => false, //
			'sMetas' => false, //
			'sTitle' => false, //
			'sCss' => false, //
			'sJs' => false, //
			);

	function __construct(){
		/*$this->cargaModelo(__CLASS__);*/

		$this->aDatos['aMod'][__CLASS__] = array('title' => $this->trad(__CLASS__));

		$this->aDatos['sMetas'] = '';
		$this->aDatos['sTitle'] = $this->aDatos['aMod'][__CLASS__]['title'];
		$this->aDatos['sCss'] .= '';
		$this->aDatos['sJs'] .= '';
	}

	//intenta instanciar el modelo de datos
	protected function cargaModelo($clase){
		$clase = $clase.'_model';
		if(!class_exists($clase)){
			$this->oModel = new $clase();
		}
	}

	//devuelve los datos que se usaran en el pintado de la plantilla
	public function getDatos(){
		return $this->aDatos;
	}

	protected function trad($cadena){
		if(function_exists('_tradR')){
			$cadena = _tradR($cadena);
		}

		return $cadena;
	}

}
