<?php
if(!defined('DUNE')) die('...');

class Portada extends Controlazo {

	function __construct(){
		parent::__construct();

		$this->cargaModelo(__CLASS__);

		$this->aDatos['aMod']['portada'] = array('title' => $this->trad('Portada'));

		$this->aDatos['sMetas'] = '<meta name="keywords" content="site.local">';
		$this->aDatos['sTitle'] = $this->aDatos['aMod']['portada']['title'];
		$this->aDatos['sCss'] .= '<style type="text/css">body{color:red;}</style>';
		$this->aDatos['sJs'] .= '';
	}

}
