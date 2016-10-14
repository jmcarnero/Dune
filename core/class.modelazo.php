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
 * @version 1
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

	/**
	 * Constructor
	 *
	 * @throws ErrorException
	 */
	public function __construct(){
		if(!defined('D_BASE_DIR') || !defined('D_DIR_LIBS')){
			throw new ErrorException('Error fatal: No se puede acceder a las librerias, faltan constantes de directorios.');
		}

		$this->sBaseDir = D_BASE_DIR . D_DIR_LIBS;

		$this->getConfig();
		$this->conectar();
	}

	/**
	 * Destructor
	 */
	public function __destruct(){
		//$this->oDDBB->desconectar();
	}

	/**
	 * Conexion a la fuente de datos
	 *
	 * @return void
	 */
	private function conectar(){
		require_once $this->sBaseDir . 'class.redatazo.inc';
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

	/**
	 * Coge los parametros de configuracion de la fuente de datos
	 *
	 * @return void
	 */
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
