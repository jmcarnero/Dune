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
 * Guarda trazas de todas las acciones en base de datos;
 * guarda las consultas que se le pasen en una tabla independiente, por defecto solo guarda insert/update/delete
 * #core PHP 4
 * Ejemplo de uso:
 * (no se puede usar directamente, solo a traves de una clase que herede de esta)
 * <code>
//definir, cuando se instancie la clase derivada:
$oTest->bTrazabilidad = true; //declara que se usaran estas funcionalidades
//$oTest->bGuardaDatos = true; //se guardara el estado anterior de los datos (update y delete); TODO no implementado
$oTest->sIdUsuario = "id_del_usuario"; //id del usuario que ejecuta la query

//echo($oTest->sError);
 * </code>
 *
 * @todo Guardar tambien estados anterior y posterior de los datos implicados.
 * @author José M. Carnero
 * @version 2.5
 * @since 2010-04-15
 * @see class.sqlazo.inc
 * @abstract
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo
 */
class SQLazo_trazabilidad extends SQLazo {

	/**
	 * Query que se va a tracear.
	 * ['query'] => consulta que se va a guardar (tracear)
	 * ['datos'] => parametros de la consulta que se va a guardar (tracear)
	 * ['cantidad'] => numero de filas a recuperar
	 * ['inicial'] => fila inicial a recuperar
	 *
	 * @var array
	 * @access private
	 */
	var $aQuerySolicitada;

	/**
	 * IP del usuario actual (decimal)
	 *
	 * @var string
	 */
	var $sIpCliente;

	/**
	 * Indica si se guardan consultas; si esta clase se usa o no; por defecto NO
	 *
	 * @var boolean
	 */
	var $bTrazabilidad;

	/**
	 * Asociativo, indica que tipos de datos y consultas se guardaran.
	 * 'insert' => boolean
	 * 'update' => boolean
	 * 'delete' => boolean
	 * 'select' => boolean
	 * 'otras' => boolean -> cualquiera que no sea parte de las anteriores
	 * 'error' => boolean -> cualquier consulta que haya generado error
	 *
	 * @var array
	 */
	var $aGuardaTipos;

	/**
	 * Indica si se guardan datos (true) o solo sentencias sql (false, por defecto) //TODO
	 *
	 * @var boolean
	 */
	var $bGuardaDatos;

	/**
	 * Identificador del usuario que haya lanzado la consulta
	 *
	 * @access private
	 * @var boolean
	 * @see SQLazo_trazabilidad::getIdUsuario(),SQLazo_trazabilidad::setIdUsuario($id)
	 */
	var $sIdUsuario;

	/**
	 * Metodo para consultas internas de la clase.
	 * Devuelve el array de resultados completo (como "SQLazo::leeFilasTodos()") o falso si ha habido error.
	 *
	 * @access protected
	 * @param string $sql Consulta SQL
	 * @return array
	 * @abstract
	 */
	protected function consultaInterna($sql){}/*{
		if(!$this->oMysqli->real_query($sql)) return(false);

		$oResultados = $this->oMysqli->use_result();
		$aResultados = array();

		if(function_exists('mysqli_result::fetch_all')) $aResultados = $oResultados->fetch_all();
		else{
			while($aResultados[] = $oResultados->fetch_array());
		}
		$oResultados->close();

		return($aResultados);
	}*/

	/**
	 * Crea las tablas necesarias si no existen.
	 *
	 * @todo Comprobar si el usuario que conecta a la base de datos tiene permiso para crear las tablas
	 * @access private
	 * @return boolean
	 */
	private function creaTablas(){
		$bRet = true;
		$sTrazaSql = '';

		//evita volver a consultar si las tablas estan creadas
		if(!isset($bCreaTablasTrazabilidad)){
			static $bCreaTablasTrazabilidad = 0;
		}
		else{
			return $bCreaTablasTrazabilidad;
		}

		$aTablas = array(
			$this->sPrefijo.'traza_sql' => false,
			$this->sPrefijo.'traza_insert' => false,
			$this->sPrefijo.'traza_update' => false,
			$this->sPrefijo.'traza_delete' => false
		);

		$aResultados = $this->consultaInterna("SHOW TABLES LIKE '" . $this->sPrefijo . "traza_%'");
		if($aResultados === false){
			$bRet = false;
		}

		for($i = 0; $i < count($aResultados); $i++){
			$aTablas[$aResultados[$i][0]] = true;
		}

		if(!$aTablas[$this->sPrefijo.'traza_sql']){ //tabla para resto de consultas
			$sTrazaSql = 'CREATE TABLE IF NOT EXISTS `'.$this->sPrefijo.'traza_sql` (`trzId` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT, `idUsuario` VARCHAR(255) NOT NULL, `ip` VARCHAR(15) NOT NULL, `fecha` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, `tipo` varchar(10) default NULL, `error` TEXT, `query` LONGBLOB NULL, PRIMARY KEY (`trzId`)) ENGINE = InnoDB;';
			if($this->consultaInterna($sTrazaSql) === false){
				$bRet = false;
			}
		}

		if($this->bGuardaDatos){ //tablas para consultas de insercion/actualizacion/borrado
			if(!$aTablas[$this->sPrefijo.'traza_insert'] && $this->aGuardaTipos['insert']){
				$sTrazaSql = 'CREATE TABLE IF NOT EXISTS `'.$this->sPrefijo.'traza_insert` (`triId` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT, `trzId` INT(11) UNSIGNED NOT NULL, `fecha` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, `datos` LONGBLOB NULL, PRIMARY KEY (`triId`), INDEX `fk_trazaInsert_trazaSql_trzId` (`trzId` ASC), CONSTRAINT `fk_trazaInsert_trazaSql_trzId` FOREIGN KEY (`trzId`) REFERENCES `_p_traza_sql` (`trzId`) ON DELETE RESTRICT ON UPDATE RESTRICT) ENGINE = InnoDB;';
				if($this->consultaInterna($sTrazaSql) === false){
					$bRet = false;
				}
			}
			if(!$aTablas[$this->sPrefijo.'traza_update'] && $this->aGuardaTipos['update']){
				$sTrazaSql = 'CREATE TABLE IF NOT EXISTS `'.$this->sPrefijo.'traza_update` (`truId` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT, `trzId` INT(11) UNSIGNED NOT NULL, `fecha` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, `datos` LONGBLOB NULL, PRIMARY KEY (`truId`), INDEX `fk_trazaUpdate_trazaSql_trzId` (`trzId` ASC), CONSTRAINT `fk_trazaUpdate_trazaSql_trzId` FOREIGN KEY (`trzId`) REFERENCES `_p_traza_sql` (`trzId`) ON DELETE RESTRICT ON UPDATE RESTRICT) ENGINE = InnoDB;';
				if($this->consultaInterna($sTrazaSql) === false){
					$bRet = false;
				}
			}
			if(!$aTablas[$this->sPrefijo.'traza_delete'] && $this->aGuardaTipos['delete']){
				$sTrazaSql = 'CREATE TABLE IF NOT EXISTS `'.$this->sPrefijo.'traza_delete` (`trdId` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT, `trzId` INT(11) UNSIGNED NOT NULL, `fecha` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, `datos` LONGBLOB NULL, PRIMARY KEY (`trdId`), INDEX `fk_trazaDelete_trazaSql_trzId` (`trzId` ASC), CONSTRAINT `fk_trazaDelete_trazaSql_trzId` FOREIGN KEY (`trzId`) REFERENCES `_p_traza_sql` (`trzId`) ON DELETE RESTRICT ON UPDATE RESTRICT) ENGINE = InnoDB;';
				if($this->consultaInterna($sTrazaSql) === false){
					$bRet = false;
				}
			}
		}

		$bCreaTablasTrazabilidad = $bRet;
		if(!$bRet){
			$this->setError(__METHOD__);
		}

		return $bRet;
	}

	/**
	 * Devuelve el identificador de usuario que se guardara en las consultas de trazabilidad
	 *
	 * @since 2012-05-17
	 * @access public
	 * @return mixed
	 */
	public function getIdUsuario(){
		return $this->sIdUsuario;
	}

	/**
	 * Guarda las consultas (y sus datos).
	 * Los parametros de la query del usuario se limpian en el proceso normal, pero la query en si puede llevar caracteres indeseados, por eso addslashes($this->sQuery) (importante a la hora de recuperar las consultas)
	 *
	 * @access protected
	 * @return boolean
	 */
	protected function guardaTraza(){
		$bRet = true;
		//$this->aQuerySolicitada = array('query' => $this->sQuery, 'datos' => $pars, 'cantidad' => $cantidad, 'inicial' => $inicial);

		$sTipo = $this->sTipoQuery;
		if(!isset($this->aGuardaTipos[$sTipo]))
			$sTipo = 'otras';

		$sError = $this->getError();
		if(!empty($sError))
			$sTipo = 'error';

		if($this->creaTablas()){
			if($this->aGuardaTipos[$sTipo]){
				$bRet = $this->consultaInterna("INSERT INTO " . $this->sPrefijo . "traza_sql (`idUsuario`, `ip`, `tipo`, `error`, `query`) VALUES ('" . $this->sIdUsuario . "', '" . $this->sIpCliente . "', '" . $sTipo . "', '" . ($this->getError() ? addslashes(implode("\n", $this->getError())) : '') . "', '" . addslashes($this->sQuery) . "')");
			}
			//else if($this->aGuardaTipos['otras']) $bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_sql (`idUsuario`, `ip`, `tipo`, `sql`) VALUES ('".$this->sIdUsuario."', '".$this->sIpCliente."', '".$sTipo."', '".addslashes($this->sQuery)."')");

			if($this->bGuardaDatos && $this->aGuardaTipos[$sTipo]){ //tablas para consultas de insercion/actualizacion/borrado
				switch($sTipo){ //TODO debe guardarse el estado anterior a update/delte, no lo que hacen las consultas actuales??
					case 'delete':
						$bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_delete (trzId, datos) VALUES (%u, '%s')", array($this->sUltimaId, serialize($pars)));
						break;
					case 'insert':
						$bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_insert (trzId, datos) VALUES (%u, '%s')", array($this->sUltimaId, serialize($pars)));
						break;
					case 'select':
						//TODO guardar resultados de una select normal?
						$bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_delete (trzId, datos) VALUES (%u, '%s')", array($this->sUltimaId, serialize($pars)));
						break;
					case 'update':
						$bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_update (trzId, datos) VALUES (%u, '%s')", array($this->sUltimaId, serialize($pars)));
						break;
					default:
						$bRet = $this->consultaInterna("INSERT INTO ".$this->sPrefijo."traza_delete (trzId, datos) VALUES (%u, '%s')", array($this->sUltimaId, serialize($pars)));
				}
			}
		}

		if($bRet === false){
			$this->setError(__METHOD__);
		}
		else{
			$bRet = true;
		}

		return $bRet;
	}

	/**
	 * Intenta devolver la IP del usuario remoto
	 *
	 * @since 2012-10-09
	 * @access private
	 * @return string
	 */
	private function ipCliente(){
		if(isset($_SERVER['HTTP_X_FORWARDED_FOR']) AND $_SERVER['HTTP_X_FORWARDED_FOR'] != ""){
			return $_SERVER['HTTP_X_FORWARDED_FOR'];
		}
		else{
			if(isset($_SERVER['REMOTE_ADDR']) AND $_SERVER['REMOTE_ADDR'] != ""){
				return $_SERVER['REMOTE_ADDR'];
			}
			else{
				return '999:999:999:999'; //ip desconocida
			}
		}
		//redes privadas ("10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16");
	}

	/**
	 * Asigna el identificador de usuario que se guardara en las consultas de trazabilidad
	 *
	 * @since 2012-05-17
	 * @access public
	 * @param mixed $clave Identificador de usuario
	 * @return boolean
	 */
	public function setIdUsuario($id = false){
		if(!isset($sIdUsuarioTrazabilidad)){
			static $sIdUsuarioTrazabilidad = 0;
		}

		//var_dump('$sIdUsuarioTrazabilidad => ', $sIdUsuarioTrazabilidad);
		if(isset($id) && $id !== false)
			$sIdUsuarioTrazabilidad = $id;

		$this->sIdUsuario = $sIdUsuarioTrazabilidad;

		return true;
	}

	/**
	 * Constructor
	 */
	function SQLazo_trazabilidad($servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = 'test', $puerto = ''){
		parent::SQLazo($servidor, $usuario, $password, $ddbb, $puerto);

		$this->aQuerySolicitada = array('query' => false, 'datos' => array(), 'cantidad' => 0, 'inicial' => 0);

		$this->bTrazabilidad = false;
		$this->aGuardaTipos = array('insert' => true, 'update' => true, 'delete' => true, 'select' => false, 'otras' => false, 'error' => true);
		$this->bGuardaDatos = false; //por defecto no se guardan datos, solo sql's

		$this->sIpCliente = $this->ipCliente();
		//$this->sIdUsuario = "0"; //desconocido
		$this->setIdUsuario();

		$this->setErrorMensaje(array(
		'SQLazo_trazabilidad::creaTablas' => 'No se han podido crear correctamente las tablas para trazabilidad.',
		'SQLazo_trazabilidad::guardaTraza' => 'No se han podido guardar trazas.'
		));
	}

	/**
	 * Concluye la transaccion, pasos posteriores a la ejecucion de la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @access protected
	 * @return boolean
	 * @abstract
	 */
	protected function transaccionFinal(){}/*{
		if($this->bTrazabilidad) $this->guardaTraza(); //guardado de trazas

		$sError = $this->getError();
		if(empty($sError)) $this->consulta('COMMIT'); //fija las querys
		else $this->consulta('ROLLBACK'); //deshace las querys
	}*/

	/**
	 * Inicia la transaccion con la base de datos, pasos previos a lanzar la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @access protected
	 * @return boolean
	 * @abstract
	 */
	protected function transaccionInicio(){}/*{
		$this->consulta('START TRANSACTION'); //inicia la transaccion
	}*/

}
