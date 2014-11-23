<?php
//namespace Sargazos.net;

/**
 * Clase modelo de datos generico
 *
 * @since 2014-11-17
 * @author jm_carnero@sargazos.net
 * @abstract
 */
abstract class Modelazo {

	private $sBaseDir;
	protected $aDatabase = array('tipo' => 'motor', 'servidor' => 'localhost', 'usuario' => 'anon', 'clave' => '', 'esquema' => 'test', 'prefijotablas' => '', 'trazabilidad' => false);
	protected $oDDBB; //objeto sqlazo

	function __construct(){
		if(defined('BASE_DIR')){
			$this->sBaseDir = BASE_DIR;
		}
		else{
			$sSalto = '/..'; //salto de la ubicacion de este fichero respecto a la raiz
			$this->sBaseDir = str_replace('\\', '/', realpath(dirname(__FILE__).$sSalto)).'/';
		}

		if(defined(DB_ENGINE)) $this->aDatabase['tipo'] = DB_ENGINE;
		if(defined(DB_SERVER)) $this->aDatabase['servidor'] = DB_SERVER;
		if(defined(DB_USER)) $this->aDatabase['usuario'] = DB_USER;
		if(defined(DB_PASSWORD)) $this->aDatabase['clave'] = DB_PASSWORD;
		if(defined(DB_DATABASE)) $this->aDatabase['esquema'] = DB_DATABASE;
		if(defined(DB_PREFIJOTABLAS)) $this->aDatabase['prefijotablas'] = DB_PREFIJOTABLAS;
		if(defined(DB_TRAZABILIDAD)) $this->aDatabase['trazabilidad'] = DB_TRAZABILIDAD;

		$this->conectar();
	}

	function __destruct(){
		$this->oDDBB->desconectar();
	}

	/*conexion a base de datos*/
	private function conectar(){
		require($this->sBaseDir.'incs/class.sqlazo.inc');
		$this->oDDBB = sqlazo_sel($this->aDatabase['tipo']);
		$this->oDDBB->conectar($this->aDatabase['servidor'], $this->aDatabase['usuario'], $this->aDatabase['clave'], $this->aDatabase['esquema']);
		$this->oDDBB->sPrefijo = $this->aDatabase['prefijotablas'];
		$this->oDDBB->bTrazabilidad = $this->aDatabase['trazabilidad'];
		$this->oDDBB->setIdUsuario(empty($_SESSION['idUsuario'])?0:$_SESSION['idUsuario']);
	}

}
