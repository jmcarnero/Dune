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

//dependencias
require_once CLASSES_DIR . '/interface.captchazo.inc';

/**
 * Creacion de CAPTCHA.
 * En lugar de imagen hace una pregunta de logica, matematica, etc.
 *
 * @author José M. Carnero
 */
class Captchazo_letra implements iCaptchazo {

	/**
	 * @var string Stiqueta HTML con el CAPTCHA y lo que necesite para integrarlo directamente en la pagina, si es falso solo se envia el propio CAPTCHA (imagen o lo que corresponda)
	 */
	private $sHtml;

	/**
	 * @var string Pregunta a responder
	 */
	private $sPregunta;

	/**
	 * @var string Texto o respuesta correcta al CAPTCHA
	 */
	private $sRespuesta;

	/**
	 * Constructor
	 */
	public function __construct(){
	}

	/**
	 * Crea el CAPTCHA
	 *
	 * @param integer $iTipo Tipo de pregunta
	 * @return boolean
	 */
	public function generar($iTipo = 0){
		$this->sRespuesta = '';

		$aMeses = array('enero', 'febrero', 'marzo', 'abril', 'mayo', 'junio', 'julio', 'agosto', 'septiembre', 'octubre', 'noviembre', 'diciembre');
		$aDias = array('lunes', 'martes', 'miércoles', 'jueves', 'viernes', 'sábado', 'domingo');
		$aAbecedario = array('a','b','c','d','e','f','g','h','i','j','k','l','m','n','p','q','r','s','t','u','v','w','x','y','z');
		$aFrases = array('Todo está perdido cuando los malos sirven de ejemplo y los buenos de mofa. Demócrito de Abdera (460 AC-370 AC)',
			'Dar ejemplo no es la principal manera de influir sobre los demás; es la única manera. Albert Einstein (1879-1955)',
			'Las palabras son enanos, los ejemplos son gigantes.',
			'Largo es el camino de la enseñanza por medio de teorías; breve y eficaz por medio de ejemplos. Lucio Anneo Séneca (2 AC-65)',
			'El ejemplo es una lección que todos los hombres pueden leer. Morris West (1916-1999)',
			'Si el prior juega a los naipes, ¿qué harán los frailes?',
			'Los ejemplos son diez veces más útiles que los preceptos. Charles James Fox (1749-1806)',
			'Los malos ejemplos son más dañinos que los crímenes. Montesquieu (1689-1755)'); //TODO permitir cargarlas desde un archivo

		if(!$iTipo){
			$iTipo = rand(2, 3);
		}

		//seleccion de pregunta/respuesta
		switch($iTipo){
			case 1: //pregunta de logica //TODO inacabado
				$this->sPregunta = '¿Cu&aacute;ntos lados tiene un triangulo is&oacute;sceles?';
				$this->sRespuesta = '3';
				break;
			case 2: //pregunta matematica
				$this->operaciones(); //TODO seleccion por parte del usuario del tipo de operacion
				break;
			case 3: //pregunta "autoalusiva" (¿cuantas r hay en esta frase?)
				$letra = $aAbecedario[rand(0, count($aAbecedario)-1)];
				$this->sPregunta = $aFrases[rand(0, count($aFrases)-1)].'<br />¿Cu&aacute;ntas "' . $letra . '" hay en todo este texto?';
				$this->sRespuesta = substr_count($this->sPregunta, $letra);
				break;
			default:
				trigger_error('Tipo solicitado no implementado', E_USER_ERROR);
				return false;
		}

		$this->sHtml = <<<HTML
	<div clas="captchazo">
		<label class="captchazoPreg" alt="CAPTCHA" />{$this->sPregunta}</label>
		<input type="text" name="captchazo" value="" />
	</div>
HTML;

		return true;
	}

	/**
	 * Devuelve HTML formado
	 *
	 * @return string
	 */
	public function getHtml(){
		return $this->sHtml;
	}

	/**
	 * Operaciones matematicas. Como texto o como simbolos.
	 *
	 * @param integer $iTipo Tipo de operacion.
	 * @return boolean
	 */
	private function operaciones($iTipo = false){
		if($iTipo === false){
			$iTipo = rand(0, 5);
		}

		$aOperandosS = array('+', '-', '*', '/');
		$pregunta = '';

		switch($iTipo){
			case 0: //¿2+2? -> 4
				array_pop($aOperandosS);
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->matheval($this->sPregunta);
				$this->sPregunta = '&iquest;'.$this->sPregunta.'?';
				break;
			case 1: //¿2+2? -> cuatro
				array_pop($aOperandosS);
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->numeroALetra($this->matheval($this->sPregunta));
				$this->sPregunta = '&iquest;'.$this->sPregunta.' (resultado en palabras, separando con espacios)?';
				break;
			case 2: //¿dos mas dos? -> 4
				array_pop($aOperandosS);
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->matheval($this->sPregunta);
				$this->sPregunta = '&iquest;'.$this->numeroALetra($this->sPregunta).'?';
				break;
			case 3: //¿dos mas dos? -> cuatro
				array_pop($aOperandosS);
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->numeroALetra($this->matheval($this->sPregunta));
				$this->sPregunta = '&iquest;'.$this->numeroALetra($this->sPregunta).' (resultado en palabras, separando con espacios)?';
				break;
			case 4: //Escriba: 2*2 (no su resultado, separando con espacios) -> dos por dos
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->numeroALetra($this->sPregunta);
				$this->sPregunta = 'Escriba: '.$this->sPregunta.' (no su resultado, separando con espacios)';
				break;
			case 5: //Escriba: dos por dos (no su resultado, sin espacios) -> 2*2
				$this->sPregunta = rand(0, 99).$aOperandosS[rand(0, count($aOperandosS)-1)].rand(0, 99);
				$this->sRespuesta = $this->sPregunta;
				$this->sPregunta = 'Escriba: '.$this->numeroALetra($this->sPregunta).' (no su resultado, sin espacios)';
				break;
			default:
				trigger_error('No se reconoce el tipo de operacion solicitado [' . $iTipo . ']', E_USER_ERROR);
				return false;
		}

		return true;
	}

	/**
	 * Convierte un numero en una secuencia de palabras: 223 -> dos dos tres
	 *
	 * @param string $sValor Valor a convertir.
	 * @return string
	 */
	private function numeroALetra($sValor){
		$aOperandos = array('+' => 'mas', '-' => 'menos', '*' => 'por', '/' => 'entre', ',' => 'coma', '.' => 'punto');
		//$aOperandosS = array('+', '-', '*', '/');
		$aNumeros = array('cero', 'uno', 'dos', 'tres', 'cuatro', 'cinco', 'seis', 'siete', 'ocho', 'nueve');

		$aTemp = array(); //array temporal con los resultados
		for($i = 0;$i < strlen($sValor);$i++){
			$temp = substr($sValor, $i, 1);
			if(is_numeric($temp)) $aTemp[] = $aNumeros[$temp];
			else $aTemp[] = $aOperandos[$temp];
		}

		return implode(' ', $aTemp);
	}

	/**
	 * Evalua una expresion matematica.
	 * cmr at expansys dot com
	 *
	 * @param string $sEquation Expresion matematica
	 * @return string
	 */
	private function matheval($sEquation){
		$sEquation = preg_replace("/[^0-9+\-.*\/()%]/","",$sEquation);
		// fix percentage calcul when percentage value < 10
		$sEquation = preg_replace("/([+-])([0-9]{1})(%)/","*(1\$1.0\$2)",$sEquation);
		// calc percentage
		$sEquation = preg_replace("/([+-])([0-9]+)(%)/","*(1\$1.\$2)",$sEquation);
		// you could use str_replace on this next line
		// if you really, really want to fine-tune this equation
		$sEquation = preg_replace("/([0-9]+)(%)/",".\$1",$sEquation);

		$sReturn = 0;
		if($sEquation != ""){
			eval("\$sReturn = " . $sEquation . ";");
		}

		return $sReturn;
	}

	/**
	 * Asigna el juego de caracteres que se usara para generar el captcha (para los tipos de captcha basados en caracteres al azar)
	 *
	 * @param string $aCaracteres Juego de caracteres
	 * @return boolean
	 */
	public function setCaracteres($aCaracteres){
		return false;
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sDirectorio Ruta del directorio
	 * @return boolean
	 */
	public function setDirectorio($sDirectorio){
		return false;
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sBaseUrl URL base en la que se encuentra la imagen captcha
	 * @return boolean
	 */
	public function setBaseUrl($sBaseUrl){
		return false;
	}

}
