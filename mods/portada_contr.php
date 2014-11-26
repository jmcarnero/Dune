<?php
if(!defined('DUNE')) die('...');

class Portada extends Controlazo {

	function __construct(){
		parent::__construct(__FILE__);

		$this->aDatos['aMod']['portada'] = array('title' => $this->trad('Portada'));

		$this->aDatos['sMetas'] = '<meta name="keywords" content="Sargazos.net">';
		$this->aDatos['sTitle'] = $this->aDatos['aMod']['portada']['title'];
		$this->aDatos['sCss'] .= '<style type="text/css">.lista_sublistazo{display:none;}</style>';
		$this->aDatos['sJs'] .= '';
	}

}
