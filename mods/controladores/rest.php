<?php if(!defined('DUNE')) die('...');

/**
 * Controlador de prueba de rest
 *
 * @author José M. Carnero
 */
class Rest extends Restazo {

	function __construct(){
		parent::__construct();
	}

	public function categoria_get($iCategoria = 0){
		$this->envia(array('metodo' => 'get!'), 200);
	}

	public function categoria_post(){
		$this->envia(array('metodo' => 'post!'), 412);
	}

}
