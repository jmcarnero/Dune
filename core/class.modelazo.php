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

	private $sBaseDir;
	private $aDatos = array(
			'tipo' => 'motor',
			//sql
			'servidor' => 'localhost',
			'usuario' => 'anon',
			'clave' => '',
			'esquema' => 'test',
			'prefijotablas' => '',
			'trazabilidad' => false,
			//csv
			'filaTitulos' => 0,
			'separadorCampos' => ','
		);

	protected $oDB = null; //objeto sqlazo

	function __construct($sLibs = ''){
		if(defined('D_BASE_DIR') && defined('D_DIR_LIBS')){
			$this->sBaseDir = D_BASE_DIR.D_DIR_LIBS;
		}
		else{
			throw new ErrorException('Error fatal: No se puede acceder a las librerias, faltan constantes de directorios.');
		}

		$this->getConfig();
		$this->conectar();
	}

	function __destruct(){
		//$this->oDDBB->desconectar();
	}

	/*conexion a la fuente de datos*/
	private function conectar(){
		/*require($this->sBaseDir.'class.sqlazo.inc'); //parametrizar nombre del fichero?
		$this->oDB = sqlazo_sel($this->aDatabase['tipo']);
		$this->oDB->conectar($this->aDatabase['servidor'], $this->aDatabase['usuario'], $this->aDatabase['clave'], $this->aDatabase['esquema']);
		$this->oDB->sPrefijo = $this->aDatabase['prefijotablas'];
		$this->oDB->bTrazabilidad = $this->aDatabase['trazabilidad'];
		$this->oDB->setIdUsuario(empty($_SESSION['idUsuario'])?0:$_SESSION['idUsuario']);/**/

		require_once $this->sBaseDir.'class.redatazo.inc';
		$this->oDB = new Redatazo(array(
				'motor' => $this->aDatos['tipo'],
				//sql
				'server' => $this->aDatos['servidor'],
				'database' => $this->aDatos['esquema'],
				'usuario' => $this->aDatos['usuario'],
				'clave' => $this->aDatos['clave'],
				//csv
				'filaTitulos' => $this->aDatos['filaTitulos'],
				'separadorCampos' => $this->aDatos['separadorCampos']
			));
	}

	/*coge los parametros de configuracion de la fuente de datos*/
	private function getConfig(){
		if(defined('D_DATOS_MOTOR')) $this->aDatos['tipo'] = D_DATOS_MOTOR; //motor de datos: mysql, csv, xml, ...

		//SQL
		if(defined('D_DATOS_SERVIDOR')) $this->aDatos['servidor'] = D_DATOS_SERVIDOR;
		if(defined('D_DATOS_USUARIO')) $this->aDatos['usuario'] = D_DATOS_USUARIO;
		if(defined('D_DATOS_CLAVE')) $this->aDatos['clave'] = D_DATOS_CLAVE;
		if(defined('D_DATOS_DATABASE')) $this->aDatos['esquema'] = D_DATOS_DATABASE;
		if(defined('D_DB_PREFIJOTABLAS')) $this->aDatos['prefijotablas'] = D_DB_PREFIJOTABLAS;
		if(defined('D_DB_TRAZABILIDAD')) $this->aDatos['trazabilidad'] = D_DB_TRAZABILIDAD;

		//CSV
		if(defined('D_DATOS_FILATITULOS')) $this->aDatos['filaTitulos'] = D_DATOS_FILATITULOS;
		if(defined('D_DATOS_SEPARADORCAMPOS')) $this->aDatos['separadorCampos'] = D_DATOS_SEPARADORCAMPOS;
	}

}
