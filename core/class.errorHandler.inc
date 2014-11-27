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

/**
 * Gestión de errores y excepciones.
 * Es recomendable incluirla antes de nada para que recoja todos los errores.
 *
 * @author José M. Carnero
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */

//Se muestran o no los errores y mensajes.
if(!defined('D_DEBUG')) define('D_DEBUG', false);

//Manejador de excepciones.
function exception_handler($exception) {
	errorHandler_sel(E_ERROR, $exception);
	//return(true); //no se puede continuar la ejecucion despues de una excepcion no controlada
}
set_exception_handler('exception_handler');

//Manejador de errores.
function error_handler($error_number, $error_string, $error_file, $error_line, $error_context){
	errorHandler_sel($error_number, $error_string, $error_file, $error_line, $error_context);
	return(true);
}
//$error_handler = set_error_handler("error_handler"); //si devuelve FALSE se continua con el gestor de errores normal
set_error_handler("error_handler");

/**
 * Manejador de errores (funcion).
 *
 * @param integer $error_number Specifies the error report level for the user-defined error. Must be a value number (required).
 * @param string $error_string Specifies the error message for the user-defined error (required).
 * @param string $error_file Specifies the filename in which the error occurred (optional).
 * @param integer $error_line Specifies the line number in which the error occurred (optional).
 * @param array $error_context Specifies an array containing every variable and their values in use when the error occurred (optional).
 */
function &errorHandler_sel($error_number, $error_string, $error_file = false, $error_line = false, $error_context = array()){
	static $obj = null;

	if($obj === null){
		error_reporting(D_DEBUG?E_ALL:E_ERROR);
		//error_reporting(E_ALL);
		ini_set('display_errors', D_DEBUG);
		//ini_set('log_errors','1');
		//ini_set('error_log', '/full/path/to/your-errors.log');

		$obj = new errorHandler();

		//here we wrap the the error in tags
		/*ini_set('error_prepend_string', '<phpfatalerror>');
		ini_set('error_append_string', '</phpfatalerror>');*/
	}

	//debe devolver TRUE para no ejecutar el gestor de errores interno de PHP
	if(!empty($error_number) && !empty($error_string)){
			if(is_object($error_string) && get_class($error_string) == 'Exception') $obj->setException($error_string, $error_number);
			else $obj->setError($error_number, $error_string, $error_file, $error_line, $error_context);
	}

	return($obj);
}

/**
 * Devuelve todos los errores producidos, o los que correspondan con una clave (como los de una clase concreta)
 *
 * @access public
 * @param string $clave Clave del grupo de errores a devolver
 * @return array
 */
function getError($clave = false){
	$oError = errorHandler_sel(0, ''); //objeto de errores

	return(call_user_func(array($oError, 'getError'), $clave));
}

/**
 * Asigna el fichero para guardar log
 *
 * @access public
 * @param string $Fichero Fichero para log
 * @return array
 */
function logFile($file = false){
	$oError = errorHandler_sel(0, ''); //objeto de errores

	return(call_user_func(array($oError, 'setLogFile'), $file));
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
class errorHandler {

	/**
	 * Backtrace del error
	 *
	 * @access private
	 * @var array
	 */
	//var $backtrace;

	/**
	 * Errores que se hayan producido
	 *
	 * @access private
	 * @var array
	 */
	var $aError;

	/**
	 * Texto del error
	 *
	 * @access private
	 * @var string
	 */
	var $sError = "";

	/**
	 * Clase/funcion en la que se ha producido el error
	 *
	 * @access private
	 * @var string
	 */
	var $sErrorClass;

	/**
	 * Ruta y nombre del fichero de log
	 * Solo se guardan en fichero si esta asignado este nombre
	 *
	 * @see errorHandler::setLogFile($file)
	 * @access private
	 * @var array
	 */
	var $sLogFile;

	/**
	 * Devuelve (return) el termino "traducido", si existe la funcion de traduccion "_tradR"
	 *
	 * @access private
	 * @param string $texto Texto a traducir
	 * @return array
	 */
	private function _t($texto){
		if(function_exists('_tradR')) $texto = _tradR($texto);

		//trigger_error($this->getMensaje(__METHOD__), E_USER_NOTICE);
		return($texto);
	}

	/*
	diz at ysagoon dot com 23-Nov-2004 10:40
	And here are my two cents for a useful and good looking backtrace function.
	*/
	private function backtrace(){
		/*$output = '<div class="backtrace">'."\n";
		$output .= "<p>Backtrace:</p><ul>\n";*/
		$output = "\tBacktrace:\n";
		$backtrace = debug_backtrace();

		if(empty($backtrace)) return(false);

		foreach($backtrace as $clave => $bt){
			//quitar los pasos con referencias a esta clase/metodos/funciones
			$bt['class'] = empty($bt['class'])?'':$bt['class'];
			$bt['function'] = empty($bt['function'])?'':$bt['function'];
			if($bt['class'] == __CLASS__ || $bt['function'] == 'error_handler' || $bt['function'] == 'errorHandler_sel') continue;

			$args = '';
			if(isset($bt['args'])){
				foreach($bt['args'] as $a){
					if(!empty($args)) $args .= ', ';

					switch (gettype($a)){
						case 'integer':
						case 'double':
							$args .= $a;
							break;
						case 'string':
							//$a = htmlspecialchars(substr($a, 0, 64)).((strlen($a) > 64) ? '...' : '');
							$args .= '"'.htmlentities($a).'"';
							break;
						case 'array':
							//$args .= 'Array('.count($a).')';
							$args .= (function_exists('json_encode')?json_encode($a):serialize($a));
							break;
						case 'object':
							$args .= 'Object('.get_class($a).')';
							break;
						case 'resource':
							$args .= 'Resource('.strstr($a, '#').')';
							break;
						case 'boolean':
							$args .= $a?'True':'False';
							break;
						case 'NULL':
							$args .= 'Null';
							break;
						default:
							$args .= 'Unknown';
					}
				}
			}

			$bt['class'] = empty($bt['class'])?'':$bt['class'];
			if($this->sErrorClass == '') $this->sErrorClass = $bt['class'];

			/*$output .= '<li>file: '.(empty($bt['line'])?'':$bt['line']).' - '.(empty($bt['file'])?'':$bt['file'])."<br />\n";
			$output .= 'call: '.$bt['class'].(empty($bt['type'])?'':$bt['type']).$bt['function'].'('.$args.")</li>\n";*/
			$output .= "\t\tcall: ".$bt['class'].(empty($bt['type'])?'':$bt['type']).$bt['function'].'('.$args.') -> line: '.(empty($bt['line'])?'':$bt['line']).' -- file: '.(empty($bt['file'])?'':$bt['file'])."\n";;
		}
		//$output .= "</ul></div>\n";
		return $output;
	}

	/**
	 * Muestra el error y lo guarda en fichero (log de errores)
	 */
	private function errorLog(){
		echo($this->sError);
		if(empty($this->sLogFile)) return(false);

		//TODO Write error to log file (CSV format)
		$errfile = fopen($this->sLogFile, 'a');
		fwrite($errfile, $this->sError."\n\n");
		fclose($errfile);
		return(true);
	}

	/**
	 * Constructor
	 *
	 * @access public
	 */
	function errorHandler(){
		$this->aError = array();
		$this->sLogFile = false;
	}

	/**
	 * Devuelve todos los errores producidos, o los que correspondan con una clave (como los de una clase concreta)
	 *
	 * @access public
	 * @param string $clave Clave del grupo de errores a devolver
	 * @return array
	 */
	public function getError($clave = false){
		if($clave === false || !isset($this->aError[$clave])) return($this->aError);
		return($this->aError[$clave]);
	}

	/**
	 * Muestra el texto asignado a un mensaje.
	 * Si no se pasa ninguna clave se muestran todos.
	 *
	 * @since 2012-09-17
	 * @access public
	 * @param $clave Clave del mensaje
	 * @return string
	 */
	public function getMensaje($clave = false){
		$aMensajes = array(
		'errorHandler::_t' => $this->_t('Traducciones no disponibles'),
		'errorHandler::setLogFile' => $this->_t('No se pueden escribir logs en')
		);

		if($clave === false) return($aMensajes);
		return($aMensajes[$clave]);
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @access private
	 * @param boolean $float Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)
	 * @return float
	 */
	private function microtimeSeg($float = false){
		list($mseg, $seg) = explode(" ", microtime());

		if($float) return((float)$mseg + (float)$seg);
		else return(date('Y-m-d H:i:s', $seg).substr($mseg, 1));
	}

	/**
	 * Crea el texto del mensaje de error
	 *
	 * @access public
	 * @param integer $error_number Specifies the error report level for the user-defined error. Must be a value number (required).
	 * @param string $error_string Specifies the error message for the user-defined error (required).
	 * @param string $error_file Specifies the filename in which the error occurred (optional).
	 * @param integer $error_line Specifies the line number in which the error occurred (optional).
	 * @param array $error_context Specifies an array containing every variable and their values in use when the error occurred (optional).
	 * @return boolean
	 */
	public function setError($error_number = false, $error_string = false, $error_file = false, $error_line = false, $error_context = false){
		if(!(error_reporting() & $error_number)){
			//Este código de error no está incluido en error_reporting
			return;
		}

		$this->sError = '';
		//$this->backtrace = array();
		$iExit = 0; //finaliza la ejecucion si != 0

		switch($error_number){
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
		//if(!empty($error_context)) $this->sError .= '<p>'.$this->_t('context').': '.$error_context."</p>\n";
		//$this->sError .= '<p>PHP '.PHP_VERSION.' ('.PHP_OS.")</p>\n";
		$this->sError = "\n** ".$this->sError.' ** ['.$error_number.']{'.$this->microtimeSeg().'} '.$error_string."\n"; //pone la fecha/hora del error
		if(!empty($error_line) || !empty($error_file)) $this->sError .= "\t".$this->_t('line').': '.$error_line.' '.$this->_t('in').' '.$this->_t('file').': '.$error_file."\n";

		$this->sErrorClass = '';
		$temp = $this->backtrace();
		if(!empty($temp)) $this->sError .= $temp;

		if($iExit) $this->sError .= '** ** '.$this->_t('stoped execution')."** **\n";

		//if($error_number & error_reporting()){
			if(empty($this->sErrorClass)) $this->aError[] = $this->sError;
			else $this->aError[$this->sErrorClass][] = $this->sError;

			//echo($this->sError);
		//}

		$this->errorLog();
		if($iExit) exit($iExit);

		//no ejecutar el gestor de errores interno de PHP
		return(true);
	}

	/**
	 * Crea el texto del mensaje de excepcion
	 *
	 * @since 2012-09-13
	 * @access public
	 * @param object $exception Objeto excepcion
	 * @param integer $error_number Se pasa como parametro para igualarlo en el singleton, pero ya que son errores fatales siempre vale E_ERROR
	 * @return boolean
	 */
	public function setException($exception, $error_number = E_ERROR){
		$this->sError = "\n## ".$this->_t('Fatal error').' ## ['.$this->_t('Uncaught exception').']{'.$this->microtimeSeg().'} '.get_class($exception).', '.$exception->getMessage()."\n"; //pone la fecha/hora del error
		$this->sError .= "\t".$this->_t('line').': '.$exception->getLine().' '.$this->_t('in').' '.$this->_t('file').': '.$exception->getFile()."\n";

		$this->sErrorClass = '';
		$temp = $this->backtrace();
		if(!empty($temp)) $this->sError .= $temp;

		//if($iExit) $this->sError .= '** ** '.$this->_t('stoped execution')."** **\n";

		if(empty($this->sErrorClass)) $this->aError[] = $this->sError;
		else $this->aError[$this->sErrorClass][] = $this->sError;

		$this->errorLog();
		return(true);
	}

	/**
	 * Recoge donde se guardara el fichero de log de errores
	 * Si no se ha asignado nada no se guarda log
	 *
	 * @since 2012-09-13
	 * @access public
	 * @param string $file Ruta y nombre del fichero de log
	 * @return boolean
	 */
	public function setLogFile($file = false){
		if(!is_writable($file)){
			if(!touch($file)){
				trigger_error($this->getMensaje(__METHOD__).': "'.$file.'"', E_USER_WARNING);
				$this->sLogFile = false;
				return(false);
			}
		}
		$this->sLogFile = $file;
		return(true);
	}

}

/*function errorHandler ($errno, $errstr, $errfile, $errline, $errcontext)
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
} // errorHandler
*/
