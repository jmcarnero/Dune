<?php
# Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>
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

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));

/**
 * Clase para conexion y querys a bases de datos Oracle
 * #core PHP 5
 * Ejemplo de uso:
 * (es preferible usar el metodo descrito en "class.sqlazo.inc")
 *
 * @see class.sqlazo.inc
 * @author José M. Carnero
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo
 */
class sqlazo_oracle extends sqlazo {

	/**
	 * Recurso de resultados
	 *
	 * @var resource
	 */
	private $rResultados;

	/**
	 * Conexion Oracle
	 *
	 * @var resource
	 */
	private $rOracle;

	/**
	 * Constructor
	 * Los parametros por defecto conectan a un servidor de Oracle local,
	 * con el usuario administrador (sin clave) y a la base de datos "dual"
	 * (que existe por defecto)
	 */
	function __construct($servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = 'dual', $puerto = 1502){
		parent::sqlazo($servidor, $usuario, $password, $ddbb, $puerto);
	}

	/**
	 * Conexion al servidor.
	 * Selecciona automaticamente la base de datos,
	 * llamar a "seleccionarDDBB()" cambiando la propiedad "$this->sDDBB"
	 * para usar otra base de datos con esta misma conexion.
	 *
	 * @param string $servidor Servidor
	 * @param string $usuario Usuario
	 * @param string $clave Clave
	 * @param string $db Schema (base de datos)
	 * @return boolean
	 */
	public function conectar($servidor = false, $usuario = false, $clave = false, $db = false, $puerto = false){
		sqlazo::instCount(); //contador de instancias

		if($servidor)
			$this->sDDBBServidor = $servidor;

		if($usuario)
			$this->sDDBBUsuario = $usuario;

		if($clave)
			$this->sDDBBPassword = $clave;

		$this->setDDBBPuerto($puerto);

		//Conecta al servicio $db (esto es, una base de datos) en la máquina "localhost"
		$this->rOracle = oci_connect($this->sDDBBUsuario, $this->sDDBBPassword, $this->sDDBBServidor . '/' . $db);

		if(!$this->rOracle){ //error en la conexion
			$e = oci_error();
			trigger_error(htmlentities($e['message'], ENT_QUOTES), E_USER_ERROR);
		}
	}

	/**
	 * Consulta a la base de datos, (si "SELECT", necesita de "sqlazo::leeFila()" para empezar a devolver resultados).
	 *
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @param integer $cantidad Numero de registros a devolver
	 * @param integer $inicial Registro inicial a devolver
	 * @return boolean
	 * @abstract
	 */
	public function consulta($query = false, $pars = array(), $cantidad = 0, $inicial = 0){
		if($query)
			$this->query($query, $pars, $cantidad, $inicial);

		$this->rResultados = oci_parse($this->rOracle, $this->sQuery);
		oci_execute($this->rResultados);
	}

	/**
	 * Desconexion del servidor.
	 *
	 * @return boolean
	 */
	public function desconectar(){
	}

	/**
	 * Lee una fila de resultados.
	 */
	public function leeFila(){
	}

	/**
	 * Crea un array con todas las filas de resultados.
	 * Los nombres de columna predeterminados de Oracle que no consideran mayúsculas/minúsculas tendrán claves de array en mayúsculas
	 *
	 * @return boolean
	 */
	public function leeFilasTodos(){
		while($aFila = oci_fetch_array($this->rResultados, OCI_ASSOC + OCI_RETURN_NULLS)){
			$this->aFilaTodos[] = $aFila;
		}

		return true;
	}

	/**
	 * Construir query.
	 *
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @param integer $cantidad Numero de registros a devolver
	 * @param integer $inicial Registro inicial a devolver
	 * @return void
	 */
	public function query($query, $pars = array(), $cantidad = 0, $inicial = 0){
		$this->sQuery = $query;
	}

}
