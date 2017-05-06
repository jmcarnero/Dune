<?php
# Copyright (C) 2011 José M. Carnero <jm_carnero@sargazos.net>
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
 * Gestión de errores y excepciones.
 * Es recomendable incluirla antes de nada para que recoja todos los errores.
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */

//Se muestran o no los errores y mensajes.
if(!defined('D_DEBUG')) define('D_DEBUG', false);

/**
 * Manejador de excepciones
 *
 * @param exception $exception Excepcion sucedida
 * @return void
 */
function exception_handler($exception) {
	errorHandler_sel(E_ERROR, $exception);
	//return(true); //no se puede continuar la ejecucion despues de una excepcion no controlada
}
set_exception_handler('exception_handler');

/**
 * Manejador de errores
 *
 * @param integer $iErrorNumber Numero de error
 * @param integer $sErrorString Texto de error
 * @param integer $sErrorFile Fichero donde ha sucedido el error
 * @param integer $iErrorLine Linea del fichero donde ha sucedido el error
 * @param integer $aErrorContext Contexto del error
 * @return boolean
 */
function error_handler($iErrorNumber, $sErrorString, $sErrorFile, $iErrorLine, $aErrorContext){
	errorHandler_sel($iErrorNumber, $sErrorString, $sErrorFile, $iErrorLine, $aErrorContext);
	return true;
}
//$error_handler = set_error_handler("error_handler"); //si devuelve FALSE se continua con el gestor de errores normal
set_error_handler('error_handler');

/**
 * Manejador de errores (funcion).
 *
 * @param integer $iErrorNumber Specifies the error report level for the user-defined error. Must be a value number (required).
 * @param string $sErrorString Specifies the error message for the user-defined error (required).
 * @param string $sErrorFile Specifies the filename in which the error occurred (optional).
 * @param integer $iErrorLine Specifies the line number in which the error occurred (optional).
 * @param array $aErrorContext Specifies an array containing every variable and their values in use when the error occurred (optional).
 * @return object
 */
function &errorHandler_sel($iErrorNumber, $sErrorString, $sErrorFile = false, $iErrorLine = false, $aErrorContext = array()){
	static $obj = null;

	if($obj === null){
		error_reporting(D_DEBUG ? E_ALL : E_ERROR);
		//error_reporting(E_ALL);
		ini_set('display_errors', D_DEBUG);
		//ini_set('log_errors','1');
		//ini_set('error_log', '/full/path/to/your-errors.log');

		$obj = new ErrorHandler();

		//here we wrap the the error in tags
		/*ini_set('error_prepend_string', '<phpfatalerror>');
		ini_set('error_append_string', '</phpfatalerror>');*/
	}

	//debe devolver TRUE para no ejecutar el gestor de errores interno de PHP
	if(!empty($iErrorNumber) && !empty($sErrorString)){
			if(is_object($sErrorString) && get_class($sErrorString) == 'Exception') $obj->setException($sErrorString, $iErrorNumber);
			else $obj->setError($iErrorNumber, $sErrorString, $sErrorFile, $iErrorLine, $aErrorContext);
	}

	return $obj;
}

/**
 * Devuelve todos los errores producidos, o los que correspondan con una clave (como los de una clase concreta)
 *
 * @access public
 * @param string $sClave Clave del grupo de errores a devolver
 * @return array
 */
function getError($sClave = false){
	$oError = errorHandler_sel(0, ''); //objeto de errores

	return call_user_func(array($oError, 'getError'), $sClave);
}

/**
 * Asigna el fichero para guardar log
 *
 * @access public
 * @param string $sFichero Fichero para log
 * @return array
 */
function logFile($sFichero = false){
	$oError = errorHandler_sel(0, ''); //objeto de errores

	return(call_user_func(array($oError, 'setLogFile'), $sFichero));
}

/**
 * Clase para gestión de errores y excepciones.
 *
 * #core PHP 4
 * Ejemplo de uso:
 * <code>
include_once("./class.errorHandler.inc");
logFile('errors.err'); //guarda en fichero ademas de mostrar por pantalla; si no se usa esta funcion solo se muestra por pantalla
trigger_error('Your error message', E_USER_NOTICE);
throw new Exception('probando excepciones');
 * </code>
 *
 * @since 2011-04-12
 */
class ErrorHandler {

	/**
	 * Backtrace del error
	 *
	 * @var array
	 */
	//private $backtrace;

	/**
	 * Errores que se hayan producido
	 *
	 * @var array
	 */
	private $aError;

	/**
	 * Texto del error
	 *
	 * @var string
	 */
	private $sError = "";

	/**
	 * Clase/funcion en la que se ha producido el error
	 *
	 * @var string
	 */
	private $sErrorClass;

	/**
	 * Ruta y nombre del fichero de log
	 * Solo se guardan en fichero si esta asignado este nombre
	 *
	 * @see ErrorHandler::setLogFile($file)
	 * @var array
	 */
	private $sLogFile;

	/**
	 * Devuelve (return) el termino "traducido", si existe la funcion de traduccion "_tradR"
	 *
	 * @param string $texto Texto a traducir
	 * @return array
	 */
	private function _t($texto){
		//if(function_exists('_tradR')) $texto = _tradR($texto);

		//trigger_error($this->getMensaje(__METHOD__), E_USER_NOTICE);
		return $texto;
	}

	/**
	 * And here are my two cents for a useful and good looking backtrace function.
	 *
	 * @author diz at ysagoon dot com 23-Nov-2004 10:40
	 * @return string
	 */
	private function backtrace(){
		/*$output = '<div class="backtrace">'."\n";
		$output .= "<p>Backtrace:</p><ul>\n";*/
		$output = "\tBacktrace:\n";
		$aBacktraces = debug_backtrace();

		if(empty($aBacktraces)) return(false);

		foreach($aBacktraces as $aBackT){
			//quitar los pasos con referencias a esta clase/metodos/funciones
			$aBackT['class'] = empty($aBackT['class']) ? '' : $aBackT['class'];
			$aBackT['function'] = empty($aBackT['function']) ? '' : $aBackT['function'];
			if($aBackT['class'] == __CLASS__ || $aBackT['function'] == 'error_handler' || $aBackT['function'] == 'errorHandler_sel'){
				continue;
			}

			$sArgs = '';
			if(isset($aBackT['args'])){
				foreach($aBackT['args'] as $mArg){
					if(!empty($sArgs)){
						$sArgs .= ', ';
					}

					switch(gettype($mArg)){
						case 'integer':
						case 'double':
							$sArgs .= $mArg;
							break;
						case 'string':
							//$mArg = htmlspecialchars(substr($mArg, 0, 64)).((strlen($mArg) > 64) ? '...' : '');
							$sArgs .= '"' . htmlentities($mArg) . '"';
							break;
						case 'array':
							//$sArgs .= 'Array('.count($mArg).')';
							$sArgs .= (function_exists('json_encode') ? json_encode($mArg) : serialize($mArg));
							break;
						case 'object':
							$sArgs .= 'Object(' . get_class($mArg) . ')';
							break;
						case 'resource':
							$sArgs .= 'Resource(' . strstr(get_resource_type($mArg), '#') . ')';
							break;
						case 'boolean':
							$sArgs .= $mArg ? 'True' : 'False';
							break;
						case 'NULL':
							$sArgs .= 'Null';
							break;
						default:
							$sArgs .= 'Unknown';
					}
				}
			}

			$aBackT['class'] = empty($aBackT['class']) ? '' : $aBackT['class'];
			if($this->sErrorClass == '') $this->sErrorClass = $aBackT['class'];

			/*$output .= '<li>file: ' . (empty($aBackT['line']) ? '' : $aBackT['line']) . ' - ' . (empty($aBackT['file']) ? '' : $aBackT['file']) . "<br />\n";
			$output .= 'call: ' . $aBackT['class'] . (empty($aBackT['type']) ? '' : $aBackT['type']) . $aBackT['function'] . '(' . $sArgs . ")</li>\n";*/
			$output .= "\t\tcall: " . $aBackT['class'] . (empty($aBackT['type']) ? '' : $aBackT['type']) . $aBackT['function'] . '(' . $sArgs . ') -> line: ' . (empty($aBackT['line']) ? '' : $aBackT['line']) . ' -- file: ' . (empty($aBackT['file']) ? '' : $aBackT['file']) . "\n";
		}
		//$output .= "</ul></div>\n";
		return $output;
	}

	/**
	 * Muestra el error y lo guarda en fichero (log de errores)
	 *
	 * @return boolean
	 */
	private function errorLog(){
		echo($this->sError);
		if(empty($this->sLogFile)){
			return false;
		}

		//TODO Write error to log file (CSV format)
		$errfile = fopen($this->sLogFile, 'a');
		fwrite($errfile, $this->sError."\n\n");
		fclose($errfile);
		return true;
	}

	/**
	 * Constructor
	 *
	 * @return void
	 */
	public function ErrorHandler(){
		$this->aError = array();
		$this->sLogFile = false;
	}

	/**
	 * Devuelve todos los errores producidos, o los que correspondan con una clave (como los de una clase concreta)
	 *
	 * @param string $sClave Clave del grupo de errores a devolver
	 * @return array
	 */
	public function getError($sClave = false){
		if($sClave === false || !isset($this->aError[$sClave])) return($this->aError);
		return $this->aError[$sClave];
	}

	/**
	 * Muestra el texto asignado a un mensaje.
	 * Si no se pasa ninguna clave se muestran todos.
	 *
	 * @since 2012-09-17
	 * @param string $sClave Clave del mensaje
	 * @return string
	 */
	public function getMensaje($sClave = false){
		$aMensajes = array(
		'ErrorHandler::_t' => $this->_t('Traducciones no disponibles'),
		'ErrorHandler::setLogFile' => $this->_t('No se pueden escribir logs en')
		);

		if($sClave === false){
			return $aMensajes;
		}

		return $aMensajes[$sClave];
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @param boolean $bFloat Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)
	 * @return float
	 */
	private function microtimeSeg($bFloat = false){
		list($mseg, $seg) = explode(" ", microtime());

		if($bFloat){
			return (float)$mseg + (float)$seg;
		}

		return date('Y-m-d H:i:s', $seg) . substr($mseg, 1);
	}

	/**
	 * Crea el texto del mensaje de error
	 *
	 * @param integer $iErrorNumber Specifies the error report level for the user-defined error. Must be a value number (required).
	 * @param string $sErrorString Specifies the error message for the user-defined error (required).
	 * @param string $sErrorFile Specifies the filename in which the error occurred (optional).
	 * @param integer $iErrorLine Specifies the line number in which the error occurred (optional).
	 * @param array $aErrorContext Specifies an array containing every variable and their values in use when the error occurred (optional).
	 * @return boolean
	 */
	public function setError($iErrorNumber = false, $sErrorString = false, $sErrorFile = false, $iErrorLine = false, $aErrorContext = false){
		if(!(error_reporting() & $iErrorNumber)){ //Este código de error no está incluido en error_reporting
			return; //TODO indicar esta situacion?
		}

		$this->sError = '';
		//$this->backtrace = array();
		$iExit = 0; //finaliza la ejecucion si != 0

		switch($iErrorNumber){
			/*case E_FATAL:
				$this->sError .= $this->_t('Fatal Error');
				$iExit = 1;
				break;*/
			case E_ERROR: //no puede ser manejado con una función definida por el usuario
				//(1) Fatal run-time errors. These indicate errors that can not be recovered from, such as a memory allocation problem. Execution of the script is halted.
				$this->sError .= $this->_t('Error');
				$iExit = 1;
				break;
			case E_WARNING:
				//(2) Run-time warnings (non-fatal errors). Execution of the script is not halted.
				$this->sError .= $this->_t('Warning');
				break;
			case E_PARSE: //no puede ser manejado con una función definida por el usuario
				//(4) Compile-time parse errors. Parse errors should only be generated by the parser.
				$this->sError .= $this->_t('Parse Error');
				break;
			case E_NOTICE:
				//(8) Run-time notices. Indicate that the script encountered something that could indicate an error, but could also happen in the normal course of running a script.
				$this->sError .= $this->_t('Notice');
				break;
			case E_CORE_ERROR: //no puede ser manejado con una función definida por el usuario
				//(16) Fatal errors that occur during PHP's initial startup. This is like an E_ERROR, except it is generated by the core of PHP. (since PHP 4)
				$this->sError .= $this->_t('Core Error');
				$iExit = 16;
				break;
			case E_CORE_WARNING: //no puede ser manejado con una función definida por el usuario
				//(32) Warnings (non-fatal errors) that occur during PHP's initial startup. This is like an E_WARNING, except it is generated by the core of PHP. (since PHP 4)
				$this->sError .= $this->_t('Core Warning');
				break;
			case E_COMPILE_ERROR: //no puede ser manejado con una función definida por el usuario
				//(64) Fatal compile-time errors. This is like an E_ERROR, except it is generated by the Zend Scripting Engine. (since PHP 4)
				$this->sError .= $this->_t('Compile Error');
				$iExit = 64;
				break;
			case E_COMPILE_WARNING: //no puede ser manejado con una función definida por el usuario
				//(128) Compile-time warnings (non-fatal errors). This is like an E_WARNING, except it is generated by the Zend Scripting Engine. (since PHP 4)
				$this->sError .= $this->_t('Compile Warning');
				break;
			case E_USER_ERROR:
				//(256) User-generated error message. This is like an E_ERROR, except it is generated in PHP code by using the PHP function trigger_error(). (since PHP 4)
				$this->sError .= $this->_t('User Error');
				$iExit = 256;
				break;
			case E_USER_WARNING:
				//(512) User-generated warning message. This is like an E_WARNING, except it is generated in PHP code by using the PHP function trigger_error(). (since PHP 4)
				$this->sError .= $this->_t('User Warning');
				break;
			case E_USER_NOTICE:
				//(1024) User-generated notice message. This is like an E_NOTICE, except it is generated in PHP code by using the PHP function trigger_error(). Default value for trigger_error(). (since PHP 4)
				$this->sError .= $this->_t('User Notice');
				break;
			case E_STRICT: //no siempre puede ser manejado con una función definida por el usuario
				//(2048) Enable to have PHP suggest changes to your code which will ensure the best interoperability and forward compatibility of your code. (since PHP 5)
				$this->sError .= $this->_t('Strict Notice');
				break;
			case E_RECOVERABLE_ERROR:
				//(4096) Catchable fatal error. It indicates that a probably dangerous error occured, but did not leave the Engine in an unstable state. If the error is not caught by a user defined handle (see also set_error_handler()), the application aborts as it was an E_ERROR. (since PHP 5.2.0)
				$this->sError .= $this->_t('Recoverable Error');
				break;
			case E_DEPRECATED:
				//(8192) Avisos en tiempo de ejecución. Habilítelo para recibir avisos sobre código que no funcionará en futuras versiones. (PHP 5.3.0)
				$this->sError .= $this->_t('Deprecated');
				break;
			case E_USER_DEPRECATED:
				//(16384) Mensajes de advertencia generados por el usuario. Son como un E_DEPRECATED, excepto que es generado por código de PHP mediante el uso de la función de PHP trigger_error() (PHP 5.3.0)
				$this->sError .= $this->_t('User deprecated');
				break;
			//case E_ALL:
				//(30719 en PHP 5.3.x, 6143 en PHP 5.2.x, 2047 anteriormente) All errors and warnings, except level E_STRICT (E_STRICT will be part of E_ALL as of PHP 6.0)
			default:
				//$this->sError .= $this->_t('Unknown error').' ('.$this->error_number.')';
				$this->sError .= $this->_t('Unknown error');
				break;
		}

		//if(!empty($aErrorContext)) $this->sError .= '<p>' . $this->_t('context') . ': ' . $aErrorContext."</p>\n";
		//$this->sError .= '<p>PHP '.PHP_VERSION.' ('.PHP_OS.")</p>\n";
		$this->sError = "\n** " . $this->sError . ' ** [' . $iErrorNumber . ']{' . $this->microtimeSeg() . '} ' . $sErrorString . "\n"; //pone la fecha/hora del error
		if(!empty($iErrorLine) || !empty($sErrorFile)){
			$this->sError .= "\t" . $this->_t('line') . ': ' . $iErrorLine . ' ' . $this->_t('in') . ' ' . $this->_t('file') . ': ' . $sErrorFile . "\n";
		}

		$this->sErrorClass = '';
		$temp = $this->backtrace();
		if(!empty($temp)){
			$this->sError .= $temp;
		}

		if($iExit){
			$this->sError .= '** ** ' . $this->_t('stoped execution') . "** **\n";
		}

		//if($iErrorNumber & error_reporting()){
		if(empty($this->sErrorClass)){
			$this->aError[] = $this->sError;
		}
		else{
			$this->aError[$this->sErrorClass][] = $this->sError;
		}

		//echo($this->sError);
		//}

		$this->errorLog();
		if($iExit){
			//exit($iExit);
			throw new Exception(__METHOD__ . ' -> ' . $this->_t('End of execution'));
		}

		//no ejecutar el gestor de errores interno de PHP
		return true;
	}

	/**
	 * Crea el texto del mensaje de excepcion
	 *
	 * @since 2012-09-13
	 * @param object $oException Objeto excepcion
	 * @param integer $iErrorNumber Se pasa como parametro para igualarlo en el singleton, pero ya que son errores fatales siempre vale E_ERROR
	 * @return boolean
	 */
	public function setException($oException, $iErrorNumber = E_ERROR){
		$iErrorNumber = E_ERROR;

		$this->sError = "\n## " . $this->_t('Fatal error') . ' ## [' . $this->_t('Uncaught exception') . ']{' . $this->microtimeSeg() . '} ' . get_class($oException) . ', ' . $oException->getMessage() . "\n"; //pone la fecha/hora del error
		$this->sError .= "\t" . $this->_t('line') . ': ' . $oException->getLine() . ' ' . $this->_t('in') . ' ' . $this->_t('file') . ': ' . $oException->getFile() . "\n";

		$this->sErrorClass = '';
		$temp = $this->backtrace();
		if(!empty($temp)){
			$this->sError .= $temp;
		}

		//if($iExit) $this->sError .= '** ** ' . $this->_t('stoped execution') . "** **\n";

		if(empty($this->sErrorClass)){
			$this->aError[] = $this->sError;
		}
		else{
			$this->aError[$this->sErrorClass][] = $this->sError;
		}

		$this->errorLog();
		return true;
	}

	/**
	 * Recoge donde se guardara el fichero de log de errores
	 * Si no se ha asignado nada no se guarda log
	 *
	 * @since 2012-09-13
	 * @param string $sFichero Ruta y nombre del fichero de log
	 * @return boolean
	 */
	public function setLogFile($sFichero = false){
		$this->sLogFile = $sFichero;

		if(!is_writable($sFichero)){
			if(!touch($sFichero)){
				trigger_error($this->getMensaje(__METHOD__).': "'.$sFichero.'"', E_USER_WARNING);
				$this->sLogFile = false;
			}
		}

		return (boolean) $this->sLogFile;
	}

}

/*function ErrorHandler ($errno, $errstr, $errfile, $errline, $errcontext)
{
 switch ($errno)
   {
      case E_USER_WARNING:
      case E_USER_NOTICE:
      case E_WARNING:
      case E_NOTICE:
      case E_CORE_WARNING:
      case E_COMPILE_WARNING:
         break;
      case E_USER_ERROR:
      case E_ERROR:
      case E_PARSE:
      case E_CORE_ERROR:
      case E_COMPILE_ERROR:
         $errorstring = "<h2>" .date('Y-m-d H:i:s') ."</h2>\n";
         $errorstring .= "<p>Fatal Error: $errstr (# $errno).</p>\n";
         $errorstring .= "<p>Error in line $errline of file '$errfile'.</p>\n";
         $errorstring .= "<p>Script: '{$_SERVER['PHP_SELF']}'.</p>\n";
         if (isset($errcontext['this'])) {
            if (is_object($errcontext['this'])) {
               $classname = get_class($errcontext['this']);
               $parentclass = get_parent_class($errcontext['this']);
               $errorstring .= "<p>Object/Class: '$classname', Parent Class: '$parentclass'.</p>\n";
            } // if
         } // if
         echo "<h2>This system is temporarily unavailable</h2>\n";
         echo "<p>The following has been reported to the administrator:</p>\n";
         echo "<b><font color='red'>\n$errorstring\n</b></font>";
         error_log($errorstring, 1, $_SERVER['SERVER_ADMIN']);
         $logfile = $_SERVER['DOCUMENT_ROOT'] .'/errorlog.html';
         error_log($errorstring, 3, $logfile);
         session_start();
         session_unset();
         session_destroy();
         die();
      default:
         break;
   } // switch
} // ErrorHandler
*/
