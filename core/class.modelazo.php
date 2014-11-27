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
 * Clase modelo de datos generico
 *
 * @author José M. Carnero
 * @since 2014-11-17
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 * @abstract
 */
abstract class Modelazo {

	//directorio de librerias de la aplicacion, asignado desde class.controlazo.php
	private $sLibs = '';

	private $sBaseDir;
	private $aDatabase = array('tipo' => 'motor', 'servidor' => 'localhost', 'usuario' => 'anon', 'clave' => '', 'esquema' => 'test', 'prefijotablas' => '', 'trazabilidad' => false);
	protected $oDB = null; //objeto sqlazo

	function __construct(){
		if(defined('D_BASE_DIR')){
			$this->sBaseDir = D_BASE_DIR;
		}
		else{
			$sSalto = '/..'; //salto de la ubicacion de este fichero respecto a la raiz
			$this->sBaseDir = str_replace('\\', '/', realpath(dirname(__FILE__).$sSalto)).'/';
		}

		if(defined('D_DB_ENGINE')) $this->aDatabase['tipo'] = D_DB_ENGINE;
		if(defined('D_DB_SERVER')) $this->aDatabase['servidor'] = D_DB_SERVER;
		if(defined('D_DB_USER')) $this->aDatabase['usuario'] = D_DB_USER;
		if(defined('D_DB_PASSWORD')) $this->aDatabase['clave'] = D_DB_PASSWORD;
		if(defined('D_DB_DATABASE')) $this->aDatabase['esquema'] = D_DB_DATABASE;
		if(defined('D_DB_PREFIJOTABLAS')) $this->aDatabase['prefijotablas'] = D_DB_PREFIJOTABLAS;
		if(defined('D_DB_TRAZABILIDAD')) $this->aDatabase['trazabilidad'] = D_DB_TRAZABILIDAD;
	}

	function __destruct(){
		//$this->oDDBB->desconectar();
	}

	/*conexion a base de datos*/
	private function conectar(){
		require($this->sBaseDir.$this->sLibs.'class.sqlazo.inc'); //parametrizar nombre del fichero?
		$this->oDB = sqlazo_sel($this->aDatabase['tipo']);
		$this->oDB->conectar($this->aDatabase['servidor'], $this->aDatabase['usuario'], $this->aDatabase['clave'], $this->aDatabase['esquema']);
		$this->oDB->sPrefijo = $this->aDatabase['prefijotablas'];
		$this->oDB->bTrazabilidad = $this->aDatabase['trazabilidad'];
		$this->oDB->setIdUsuario(empty($_SESSION['idUsuario'])?0:$_SESSION['idUsuario']);
	}

	/*
	 * Realiza consultas a la base de datos
	 * devuelve null si no se puede hacer la consulta
	 *
	 * @param string @query Consulta SQL
	 * @return recordset
	 */
	protected function consulta($query = false){
		if($this->oDB == null) $this->conectar();

		if(empty($query)) return null;
	}

	/**
	 * Asigna el directorio de la bibliotecas, asignado desde class.controlazo.php
	 */
	public function setDirectorioLibs($sLibs){
		if(!empty($sLibs)) $this->sLibs = $sLibs;
	}
}
