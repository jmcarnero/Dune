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

require_once(CLASSES_DIR . "/interface.sqlazo.inc");

/**
 * Clase de consultas a bases de datos mediante PDO (Php Data Objects)
 *
 * Puede funcionar de forma completamente independiente
 * PHP >= 5
 * Ejemplo de uso:
 * <code>
require_once 'interface.sqlazo.inc';
require 'class.sqlazo_pdo.inc';

try{
	$aParametros = array('motor' => 'mysql', 'servidor' => 'localhost', 'puerto' => '3306', 'esquema' => 'test', 'usuario' => 'dbUser', 'clave' => '?');
	$oDb = new SQLazo_pdo($aParametros);
	//$oDb->setMotor($aParametros); //solo es necesario para cambiar parametros o si no se han asignado con el constructor
	//$oDb->setConexion($aParametros); //solo es necesario para cambiar parametros o si no se han asignado con el constructor

	$sSql = 'SELECT id, texto FROM tabla WHERE texto LIKE ?';
	$aParametros = array('%patata');

	$aResultados = $oDb->consulta($sSql, $aParametros);
}
catch(PDOException $pdoEx){
	var_dump('Error: ', $pdoEx->getMessage());
}

for($i = 0; $i < $oDb->totalFilas(); $i++){
	var_dump($aResultados[$i]->texto);
}
 * </code>
 *
 * @author José M. Carnero
 * @since 2016-08-24
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo_pdo
 */
class SQLazo_pdo implements iSQLazo {

	private $oDB; //objeto actual de base de datos
	private $oStatement; //objeto con consulta preparada

	private $sDbBaseDatos; //base de datos en la que lanzar las consultas //para sqlite es el nombre del archivo de base de datos: database.db
	private $sDbCharset; //juego de caracteres utilizado en las conexiones a la base de datos
	private $sDbClave; //clave del usuario que conecta a la base de datos
	private $sDbMotor; //motor de base de datos
	private $sDbPuerto; //puerto del servidor al que conectar
	private $sDbServidor; //servidor de base de datos //para sqlite es la ruta donde encontrar el archivo de base de datos: my/database/path/
	private $sDbUsuario; //nombre o identificador del usuario que conecta a la base de datos
	private $iTipoResultados; //tipo de resultados a devolver

	private $iFilasCantidad = null;
	private $iFilasPagina = null;

	private $iFilasAfectadas; //filas afectadas por la ultima consulta (numero de filas en select, filas tocadas en insert/update/delete)
	private $aSqlParametros; //ultimos parametros utilizados con la ultima consulta, tal como hayan sido proporcionadas a la clase
	private $sSqlUltima; //ultima SQL lanzada
	private $iUltimoId; //ultimo id insertado (solo tiene sentido en consultas INSERT sobre tablas con clave autonumerica)

	/**
	 * Constructor
	 *
	 * @param array $aParametros Parametros de conexion y configuracion de la base de datos
	 */
	public function __construct($aParametros = array()){
		$this->oDB = null;
		$this->oStatement = null;
		$this->aAtributosConexion = array();

		$this->iFilasAfectadas = 0;
		$this->iUltimoId = null;

		$this->sSqlUltima = null;
		$this->aSqlParametros = array();

		$this->sUltimaConsulta = null;

		$this->setMotor($aParametros);
		$this->setConexion($aParametros);
		$this->setTipoResultados();

		$this->_conexion();
	}

	/**
	 * Destructor
	 */
	public function __destruct(){
		# close the connection
		$this->oDB = null;
	}

	/**
	 * Establece la conexion a la base de datos
	 *
	 * @throws PDOException
	 * @return void
	 */
	private function _conexion(){
		switch($this->sDbMotor){
			case 'sqlite': //SQLite 3 Databases
				$this->oDB = new PDO($this->sDbMotor . ':' . $this->sDbServidor . $this->sDbBaseDatos);
				break;
			default:
				$this->oDB = new PDO($this->sDbMotor . ':host=' . $this->sDbServidor . ';dbname=' . $this->sDbBaseDatos . ';charset=' . $this->sDbCharset, $this->sDbUsuario, $this->sDbClave);
		}

		$this->oDB->setAttribute(PDO::ATTR_EMULATE_PREPARES, false); //solo util con versiones viejas de MySQL

		//$this->oDB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT); //This is the default error mode. If you leave it in this mode, you'll have to check for errors in the way you're probably used to if you used the mysql or mysqli extensions.
		//$this->oDB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING); //This mode will issue a standard PHP warning, and allow the program to continue execution. It's useful for debugging.
		$this->oDB->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION); //This is the mode you should want in most situations. It fires an exception, allowing you to handle errors gracefully and hide data that might help someone exploit your system.
	}

	/**
	 * Lanza una consulta y devuelve sus resultados
	 *
	 * @throws PDOException
	 * @param string $sConsulta Consulta a preparar
	 * @param array $aParametros Parametros para la consulta, en el mismo orden que aparezcan en la SQL; se sustituira cada ? por el parametro en la posicion correspondiente
	 * @param integer $iCantidad Numero de registros a devolver
	 * @param integer $iPagina Pagina desde la que devolver
	 * @return void
	 */
	public function consulta($sConsulta, $aParametros = array(), $iCantidad = 0, $iPagina = 0){
		$this->sSqlUltima = $sConsulta;
		$this->aSqlParametros = $aParametros;
		$this->iFilasCantidad = $iCantidad;
		$this->iFilasPagina = $iPagina;

		$this->_preparaConsulta();

		return $this->_resultados();
	}

	/**
	 * Devuelve el total de resultados independiente de LIMIT
	 *
	 * @todo construir para otros motores ademas de mysql
	 */
	private function _cuentaFilas(){
		$this->iFilasAfectadas = null; //valor devuelto para motores no soportados

		if($this->sDbMotor == 'mysql'){
			$this->iFilasAfectadas = $this->oStatement->rowCount();

			if(strpos($this->sSqlUltima, 'SQL_CALC_FOUND_ROWS') !== false && strpos($this->sSqlUltima, 'LIMIT') !== false){ //calculo de resultados para consultas con LIMIT
				$oCuentaStmt = $this->oDB->query('SELECT FOUND_ROWS() AS totalRows', PDO::FETCH_ASSOC);
				$aCuentaResultados = $oCuentaStmt->fetchAll();
				$this->iFilasAfectadas = (int) $aCuentaResultados[0]['totalRows'];
				$oCuentaStmt->closeCursor();
			}
		}
	}

	/**
	 * Devuelve un array con informacion de la conexion
	 *
	 * Atributos PDO modificables
	 *
	 * @return array
	 */
	public function info(){
		if(empty($this->aAtributosConexion) && !empty($this->oDB)){
			$sAtributosFijo = 'PDO::ATTR_'; //parte fija del nombre del atributo
			$aAtributosVar = array(
				'AUTOCOMMIT',
				'CASE',
				'CLIENT_VERSION',
				'CONNECTION_STATUS',
				'DRIVER_NAME',
				'ERRMODE',
				'ORACLE_NULLS',
				'PERSISTENT',
				'PREFETCH',
				'SERVER_INFO',
				'SERVER_VERSION',
				'TIMEOUT'
			);

			foreach($aAtributosVar as $sAttr){
				$this->aAtributosConexion[$sAttr] = $this->oDB->getAttribute(constant($sAtributosFijo . $sAttr));
			}
		}

		return $this->aAtributosConexion;
	}

	/**
	 * Establece la clausula LIMIT (o lo que corresponda segun el motor de base de datos)
	 *
	 * @todo comprobar que en la consulta no exista clausula LIMIT; y que sea consulta tipo SELECT
	 */
	private function _limit(){
		$this->iFilasCantidad = (int) $this->iFilasCantidad;
		$this->iFilasPagina = (int) $this->iFilasPagina;

		if($this->iFilasCantidad){
			//si "$inicial=0" devuelve "$cantidad" de registros desde el primero
			//si se quieren devolver todos los que haya desde un "$inicial=x", dar como "$cantidad" un valor mayor al total de registros que pueda devolver la consulta (un numero al azar suficientemente grande, por ejemplo 123456789123456789123456789)
			switch($this->sDbMotor){
				case 'mysql':
					$this->sSqlUltima .= "\nLIMIT " . $this->iFilasPagina . ',' . $this->iFilasCantidad;

					break;
				case 'mssql':
				case 'sybase':
				case 'pgsql':
				case 'sqlite':
				default:
					//LIMIT desconocido, no existente o no implementado
			}
		}
	}

	/**
	 * @throws PDOException
	 * @return void
	 */
	private function _preparaConsulta(){
		//reiniciar valores de resultados de consulta
		$this->iFilasAfectadas = 0;
		$this->iUltimoId = null;

		$this->_limit();

		//suma a la consulta "SQL_CALC_FOUND_ROWS", permite tener el total de filas en consultas limitadas con "SELECT FOUND_ROWS()", sin volver a lanzar la consulta
		if($this->sDbMotor == 'mysql' && $this->iFilasCantidad){
			$iSelectPos = strpos(strtoupper($this->sSqlUltima), 'SELECT ');
			if($iSelectPos !== false && strpos($this->sSqlUltima, 'SQL_CALC_FOUND_ROWS') === false){ //TODO mejorar deteccion de tipo de consulta
				$this->sSqlUltima = substr($this->sSqlUltima, 0, $iSelectPos + 7).'SQL_CALC_FOUND_ROWS '.substr($this->sSqlUltima, $iSelectPos + 7);
			}
		}

		$this->oStatement = $this->oDB->prepare($this->sSqlUltima);

		//unnamed placeholders: $this->oStatement = $this->oDB->prepare("INSERT INTO folks (name, addr, city) values (?, ?, ?);
		//						$this->oStatement->bindParam(1, 'patata', PDO::PARAM_STR);
		//						$this->oStatement->bindValue(2, 5, PDO::PARAM_INT);
		//named placeholders: 	$this->oStatement = $this->oDB->prepare("INSERT INTO folks (name, addr, city) value (:name, :addr, :city)");
		//						$this->oStatement->bindParam(':name', 'patata', PDO::PARAM_STR);
		//para like:		 	$this->oStatement = $this->oDB->prepare("SELECT field FROM table WHERE field LIKE ?");
		//						$this->oStatement->bindParam(1, '%patata%', PDO::PARAM_STR);
		//tambien:
		//						$data = array('name' => 'Cathy', 'addr' => '9 Dark and Twisty', 'city' => 'Cardiff');
		//						$this->oStatement = $this->oDB->prepare("INSERT INTO folks (name, addr, city) value (:name, :addr, :city)");
		//						$this->oStatement->execute($data);

		$this->oStatement->execute($this->aSqlParametros);

		//recupera informacion de la ultima consulta enviada en la instruccion anterior (execute)
		/*ob_start();
		$this->oStatement->debugDumpParams();
		$this->sSqlUltima = ob_get_contents();
		ob_end_clean();*/

		$this->iUltimoId = $this->oDB->lastInsertId();
		$this->_cuentaFilas();
	}

	/**
	 * Recuperacion de resultados
	 */
	private function _resultados(){
		$aResultados = array();

		$iSelectPos = strpos(strtoupper($this->sSqlUltima), 'SELECT ');
		if($iSelectPos !== false){ //no hay resultados que devolver si no se pide una cantidad de filas (consultas SELECT) //FIXME puede dar problemas con subconsultas
			$this->oStatement->setFetchMode($this->iTipoResultados);

			//showing the results
			$aResultados = $this->oStatement->fetchAll();
			$this->oStatement->closeCursor();
		}

		return $aResultados;
	}

	/**
	 * Deben pasarse todos los parametros cada vez que sea llamada, las claves no pasadas tomaran el valor por defecto
	 *
	 * @param array $aParametros Parametros de conexion a la base de datos, claves: "servidor", "puerto", "esquema" (base de datos, schema), "charset" (juego de caracteres de la conexion), "usuario", "clave"
	 * @return void
	 */
	public function setConexion($aParametros = array()){
		if(!isset($aParametros['servidor'])){
			$aParametros['servidor'] = 'localhost';
		}

		if(empty($aParametros['puerto']) || abs((int) $aParametros['puerto']) > 65535){
			$aParametros['puerto'] = 3306; //puerto por defecto de MySQL
		}
		else{
			$aParametros['puerto'] = abs((int) $aParametros['puerto']);
		}

		if(!isset($aParametros['esquema'])){
			$aParametros['esquema'] = 'test';
		}

		if(!isset($aParametros['charset'])){
			$aParametros['charset'] = 'utf8';
		}

		if(!isset($aParametros['usuario'])){
			$aParametros['usuario'] = 'dbUser';
		}

		if(!isset($aParametros['clave'])){
			$aParametros['clave'] = '?';
		}

		$this->sDbServidor = $aParametros['servidor'];
		$this->sDbPuerto = $aParametros['puerto'];
		$this->sDbBaseDatos = $aParametros['esquema'];
		$this->sDbCharset = $aParametros['charset'];
		$this->sDbUsuario = $aParametros['usuario'];
		$this->sDbClave = $aParametros['clave'];
	}

	/**
	 * Asigna el motor de base de datos que se usuara en las consultas
	 *
	 * @throws PDOException
	 * @param array/string $sMotor Motor de base de datos; si se pasa como array se intenta usar el contenido de la clave "motor"
	 * @return void
	 */
	public function setMotor($sMotor = 'mysql'){
		if(is_array($sMotor)){
			$sMotor = empty($sMotor['motor']) ? 'mysql' : $sMotor['motor'];
		}

		switch($sMotor){
			case 'mssql': //MS SQL Server with PDO_DBLIB
			case 'sybase': //Sybase with PDO_DBLIB
				$this->sDbMotor = 'dblib';
				break;
			case 'mysql': //MySQL with PDO_MYSQL
			case 'pgsql': //PostgreSQL
			case 'sqlite': //SQLite 3 Databases
				$this->sDbMotor = $sMotor;
				break;
			default:
				throw new PDOException('Motor de base de datos desconocido');
		}

		if(!in_array($this->sDbMotor, PDO::getAvailableDrivers(), true)){
			throw new PDOException('Motor de base de datos no disponible o no configurado');
		}
	}

	/**
	 * Asigna como se devolveran los resultados
	 *
	 * Por defecto como un array de objetos, siendo cada fila devuelta un objeto
	 *
	 * @param string $sTipo Tipo a devolver
	 * @return boolean
	 */
	public function setTipoResultados($sTipo = 'obj'){
		//$this->oStatement->setFetchMode(PDO::FETCH_ASSOC); //returns an array indexed by column name
		//$this->oStatement->setFetchMode(PDO::FETCH_BOTH); //(default) returns an array indexed by both column name and number
		//$this->oStatement->setFetchMode(PDO::FETCH_BOUND); //Assigns the values of your columns to the variables set with the ->bindColumn() method
		//$this->oStatement->setFetchMode(PDO::FETCH_CLASS); //Assigns the values of your columns to properties of the named class. It will create the properties if matching properties do not exist
		//$this->oStatement->setFetchMode(PDO::FETCH_INTO); //Updates an existing instance of the named class
		//$this->oStatement->setFetchMode(PDO::FETCH_LAZY); //Combines PDO::FETCH_BOTH/PDO::FETCH_OBJ, creating the object variable names as they are used
		//$this->oStatement->setFetchMode(PDO::FETCH_NUM); //returns an array indexed by column number
		//$this->oStatement->setFetchMode(PDO::FETCH_OBJ); //returns an anonymous object with property names that correspond to the column names

		switch($sTipo){
			case 'aso': //array de arrays asociativos
				$this->iTipoResultados = PDO::FETCH_ASSOC;
				break;
			case 'both': //array de arrays asociativos y numericos
				$this->iTipoResultados = PDO::FETCH_BOTH;
				break;
			case 'num': //array de arrays numericos
				$this->iTipoResultados = PDO::FETCH_NUM;
				break;
			case 'obj': //array de objetos
			default: //tipo no reconocido
				$this->iTipoResultados = PDO::FETCH_OBJ;
		}

		return true;
	}

	/**
	 * Devuelve el numero de filas afectadas por la ultima consulta
	 *
	 * Para consultas SELECT devuelve el numero de filas recuperadas (el forzado con LIMIT si fuera el caso)
	 *
	 * @return integer
	 */
	public function totalFilas(){
		return $this->iFilasAfectadas;
	}

	/**
	 * Muestra la ultima consulta lanzada y sus parametros
	 *
	 * return string
	 */
	public function ultimaConsulta(){
		$this->sUltimaConsulta = str_replace('SQL_CALC_FOUND_ROWS ', '', $this->sSqlUltima); //no es necesario despues de calcular el total de filas y no es algo puesto por el usuario

		$bNumerados = $this->aSqlParametros == array_values($this->aSqlParametros);
		foreach($this->aSqlParametros as $sClave => $sValor){
			if(is_string($sValor)){
				$sValor = "'$sValor'";
			}

			$this->sUltimaConsulta = preg_replace('/\?/', $sValor, $this->sUltimaConsulta, 1);
			if(!$bNumerados){
				$this->sUltimaConsulta = str_replace(":$sClave", $sValor, $this->sUltimaConsulta);
			}
		}

		return $this->sUltimaConsulta;
/* $this->oStatement->queryString recoge el texto (para parametros con nombre):
SQL: [96] SELECT name, colour, calories
    FROM fruit
    WHERE calories < :calories AND colour = :colour
Params:  2
Key: Name: [9] :calories
paramno=-1
name=[9] ":calories"
is_param=1
param_type=1
Key: Name: [7] :colour
paramno=-1
name=[7] ":colour"
is_param=1
param_type=2

(para parametros sin nombre):
SQL: [82] SELECT name, colour, calories
    FROM fruit
    WHERE calories < ? AND colour = ?
Params:  2
Key: Position #0:
paramno=0
name=[0] ""
is_param=1
param_type=1
Key: Position #1:
paramno=1
name=[0] ""
is_param=1
param_type=2
*/
	}

	/**
	 * Devuelve el ultimo Id (en consultas INSERT con clave primaria autonumerica) insertado
	 *
	 * @return integer
	 */
	public function ultimoId(){
		return $this->iUltimoId;
	}

}

