<?php
/*Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
http://www.gnu.org/copyleft/gpl.html*/

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));

/**
 * Clase para conexion y consultas a bases de datos.
 *
 *
 * \code{.php}
include_once("./class.sqlazo.inc");

$oSQLazo = new SQLazo('localhost', 'test', 'test', 'test', 'mysql');
$oSQLazo->consulta('SELECT id, texto FROM tabla WHERE texto LIKE ?', array('test'), 6, 0);

var_dump($oSQLazo->leeFilasTodas());
 * \endcode
 *
 * @author José M. Carnero
 * @version 3b
 * @since 2008-03-01
 * @abstract
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo
 */
class SQLazo {

	/**
	 * Todas las filas de resultados
	 *
	 * @var array
	 */
	private $aFilas;

	/**
	 * Parametros de la consulta
	 *
	 * @see SQLazo::consulta($query, $pars, $cantidad, $inicial)
	 * @var array
	 */
	private $aParametros;

	/**
	 * SQL query completa; se envia a las funciones como cadena, cada "?" implica una sustitucion con un valor del array de parametros
	 *
	 * @see SQLazo::consulta($sSql, $aParams, $iCantidad, $iInicial)
	 * @var string
	 */
	private $sConsulta;

	/**
	 * Subobjeto de consultas
	 *
	 * @var object
	 */
	private $oSQLazo;

	/**
	 * Constructor
	 *
	 * Intenta conectar al servidor indicado
	 *
	 * @param string $sServidor Servidor
	 * @param string $sUsuario Usuario
	 * @param string $sPassword Clave
	 * @param string $sDdbb Base de datos
	 * @param string $sMotor Motor de base de datos (mysql, mssql, pgsql, sqlite)
	 * @param string $iPuerto Puerto
	 * @return void
	 */
	public function __construct($sServidor = 'localhost', $sUsuario = 'test', $sPassword = 'test', $sDdbb = 'test', $sMotor = 'mysql', $iPuerto = ''){
		$this->aFila = array();
		$this->aFilaTodos = array();

		require_once(CLASSES_DIR . '/class.sqlazo_pdo.inc');

		try{
			$aParametros = array('motor' => $sMotor, 'servidor' => $sServidor, 'puerto' => $iPuerto, 'esquema' => $sDdbb, 'usuario' => $sUsuario, 'clave' => $sPassword);
			$this->oSQLazo = new SQLazo_pdo($aParametros);
			//$this->oSQLazo->setMotor($aParametros); //solo es necesario para cambiar parametros o si no se han asignado con el constructor
			//$this->oSQLazo->setConexion($aParametros); //solo es necesario para cambiar parametros o si no se han asignado con el constructor
		}
		catch(PDOException $pdoEx){
			var_dump('Error: ', $pdoEx->getMessage());
		}
	}

	/**
	 * Consulta a la base de datos
	 *
	 * @param string $sSql Query SQL
	 * @param array $aParams Parametros de la query
	 * @param integer $iCantidad Numero de registros a devolver
	 * @param integer $iInicial Registro inicial a devolver
	 * @return boolean
	 */
	public function consulta($sSql = false, $aParams = array(), $iCantidad = 0, $iInicial = 0){
		try{
			//$sSql = 'SELECT id, texto FROM tabla WHERE texto LIKE ?';

			$this->aFilas = $this->oSQLazo->consulta($sSql, $aParams, $iCantidad, $iInicial);
		}
		catch(PDOException $pdoEx){
			var_dump('Error: ', $pdoEx->getMessage());
		}

		return true;
	}

	/**
	 * Lee una fila de resultados.
	 *
	 * @return boolean
	 */
	public function leeFila(){
		return $this->aFilas[$i];
	}

	/**
	 * Crea un array con todas las filas de resultados.
	 *
	 * @return boolean
	 */
	public function leeFilaTodas(){
		return $this->aFilas;
	}

	/**
	 * Asigna como se devolveran los resultados
	 *
	 * @param string $sTipo Tipo a devolver: aso, both, num, obj
	 * @return boolean
	 */
	public function setTipoResultados($sTipo){
		return $this->oSQLazo->setTipoResultados($sTipo);
	}

	/**
	 * Total de filas recuperadas
	 *
	 * @return integer
	 */
	public function totalFilas(){
		return $this->oSQLazo->totalFilas();
	}

	/**
	 * Ultima consulta lanzada
	 *
	 * @return string
	 */
	public function ultimaConsulta(){
		return $this->oSQLazo->ultimaConsulta();
	}

	/**
	 * Ultimo ID generado en insercion (para tablas con id autonumerico o similar)
	 *
	 * @return integer
	 */
	public function ultimoId(){
		return $this->oSQLazo->ultimoId();
	}

}
