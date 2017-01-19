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

/**
 * Creacion y uso de CAPTCHA
 *
 * @author José M. Carnero
 * @version 2b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Captchazo
 */

/**
 * Clase de funciones graficas, no instanciar directamente.
 *
 */
abstract class Captchazo_funGraficas {

	/**
	 * @var integer Alto (en pixeles)
	 */
	protected $iAlto;

	/**
	 * @var integer Ancho (en pixeles)
	 */
	protected $iAncho;

	/**
	 * @var string Url base en la que se encuentra la imagen captcha (para devolver HTML)
	 */
	protected $sBaseUrl = './';

	/**
	 * @var array Color de fondo (RGB)
	 */
	protected $aColorFondo;

	/**
	 * @var array Color del texto (RGB)
	 */
	protected $aColorTexto;

	/**
	 * @var string Ruta fisica completa con barra final, debe ser escribible, "chmod 777" o lo que corresponda
	 */
	protected $sDirectorio;

	/**
	 * @var integer Version de la libreria GD, falso si no existe
	 */
	protected $iGDVersion;

	/**
	 * @var resource Recurso de la imagen generada
	 */
	protected $rImagen;

	/**
	 * @var string Nombre completo de la imagen (sin ruta)
	 */
	protected $sImagenNombre = null;

	/**
	 * @var string Pregunta a responder
	 */
	protected $sPregunta;

	/**
	 * @var string Texto o respuesta correcta al CAPTCHA
	 */
	protected $sRespuesta;

	/**
	 * Constructor
	 */
	function __construct(){

		//obtener version de libreria GD (pone propiedad a falso si no hay libreria, condicionales con ===)
		if(!extension_loaded('gd')){
			$this->iGDVersion = false;
			trigger_error('La librería GD no se encuentra disponible, es necesaria para las funciones gráficas', E_USER_WARNING);
		}
		else{
			$aGDinfo = gd_info();
			$this->iGDVersion = (int) substr(preg_replace('/[^\d\.]+/', '', $aGDinfo['GD Version']), 0, 1);
		}

		$this->rImagen = null;
		$this->sDirectorio = null; //'/tmp/';
		$this->sPregunta = '';
		$this->sRespuesta = '';
		$this->iAncho = 200;
		$this->iAlto = 100;
		$this->aColorTexto = array(rand(0,127),rand(0,127),rand(0,127)); //crea un color RGB usando la primera mitad de la paleta
		$this->aColorFondo = array(rand(128,255),rand(128,255),rand(128,255)); //crea un color RGB usando la segunda mitad de la paleta

		$this->sImagenNombre = 'captchazo.png'; //nombre por defecto

		//constantes para la funcion "imagefilter" (por si no existe)
		//tipo de filtro
		defined('IMG_FILTER_NEGATE') or define('IMG_FILTER_NEGATE', 0); //Reverses all colors of the image
		defined('IMG_FILTER_GRAYSCALE') or define('IMG_FILTER_GRAYSCALE', 1); //Converts the image into grayscale
		defined('IMG_FILTER_BRIGHTNESS') or define('IMG_FILTER_BRIGHTNESS', 2); //Changes the brightness of the image. Use arg1 to set the level of brightness. The range for the brightness is -255 to 255
		defined('IMG_FILTER_CONTRAST') or define('IMG_FILTER_CONTRAST', 3); //Changes the contrast of the image. Use arg1 to set the level of contrast
		defined('IMG_FILTER_COLORIZE') or define('IMG_FILTER_COLORIZE', 4); //Like IMG_FILTER_GRAYSCALE, except you can specify the color. Use arg1, arg2 and arg3 in the form of red, green, blue and arg4 for the alpha channel. The range for each color is 0 to 255
		defined('IMG_FILTER_EDGEDETECT') or define('IMG_FILTER_EDGEDETECT', 5); //Uses edge detection to highlight the edges in the image
		defined('IMG_FILTER_EMBOSS') or define('IMG_FILTER_EMBOSS', 6); //Embosses the image
		defined('IMG_FILTER_GAUSSIAN_BLUR') or define('IMG_FILTER_GAUSSIAN_BLUR', 7); //Blurs the image using the Gaussian method
		defined('IMG_FILTER_SELECTIVE_BLUR') or define('IMG_FILTER_SELECTIVE_BLUR', 8); //Blurs the image
		defined('IMG_FILTER_MEAN_REMOVAL') or define('IMG_FILTER_MEAN_REMOVAL', 9); //Uses mean removal to achieve a "sketchy" effect.
		defined('IMG_FILTER_SMOOTH') or define('IMG_FILTER_SMOOTH', 10); //Makes the image smoother. Use arg1 to set the level of smoothness
		defined('IMG_FILTER_PIXELATE') or define('IMG_FILTER_PIXELATE', 11); //Applies pixelation effect to the image, use arg1 to set the block size and arg2 to set the pixelation effect mode
	}

	/**
	 * Crea la imagen base.
	 *
	 * @todo permitir varios tipos de imagenes, no solo png
	 * @param integer $iAncho Ancho de la imagen
	 * @param integer $iAlto Alto de la imagen
	 * @return boolean
	 */
	protected function creaImagen($iAncho = 0, $iAlto = 0){
		if($iAncho){
			$this->iAncho = $iAncho;
		}
		if($iAlto){
			$this->iAlto = $iAlto;
		}

		if($this->iGDVersion === false){
			return false; //no se pueden crear imagenes sin esta libreria //TODO metodos alternativos para crear imagenes (imagemagic, ...)
		}

		if($this->sDirectorio !== false && $this->sDirectorio !== 'base64'){
			@unlink($this->sDirectorio . $this->sImagenNombre); //borra la imagen previa, si existe
		}

		if($this->iGDVersion >= 2){
			$this->rImagen = imagecreatetruecolor($this->iAncho, $this->iAlto);
		}
		else{
			$this->rImagen = imagecreate($this->iAncho, $this->iAlto);
		}
		$iColorFondo = imagecolorallocate($this->rImagen, $this->aColorFondo[0], $this->aColorFondo[1], $this->aColorFondo[2]);
		imagefill($this->rImagen, 0, 0, $iColorFondo);

		//$iColorTexto = imagecolorallocate($this->rImagen, $this->aColorTexto[0], $this->aColorTexto[1], $this->aColorTexto[2]);
		if($this->rImagen === false){
			trigger_error('No se ha podido crear la imagen', E_USER_ERROR);
			return false;
		}

		return true;
	}

	/**
	 * Espejo de la imagen.
	 *
	 * @return boolean
	 */
	protected function espejo(){
		$rImagenAux = imagecreatetruecolor($this->iAncho, $this->iAlto);

		for($i = 0; $i < $this->iAncho; $i++){
			for($j = 0; $j < $this->iAlto; $j++){
				$ref = imagecolorat($this->rImagen, $i, $j);
				imagesetpixel($rImagenAux, $this->iAncho - $i, $j, $ref);
			}
		}

		imagedestroy($this->rImagen);
		$this->rImagen = &$rImagenAux;

		return true;
	}

	/**
	 * Devuelve la pregunta
	 *
	 * @return string
	 */
	public function getPregunta(){
		return $this->sPregunta;
	}

	/**
	 * Devuelve la respuesta
	 *
	 * @return string
	 */
	public function getRespuesta(){
		return $this->sRespuesta;
	}

	/**
	 * Guarda la imagen.
	 *
	 * @return boolean
	 */
	protected function guardaImagen(){
		$bRet = false;

		//no se ha creado imagen
		if($this->rImagen === false){
			trigger_error('No se puede guardar la imagen, no se ha podido crear la imagen', E_USER_ERROR);
			$bRet = false;
		}
		else{
			if(empty($this->sDirectorio)){ //salida de la imagen directamente al navegador

				function output_handler($oImg){ //permite calcular la longitud del fichero que se envia
					header('Content-Length: ' . strlen($oImg));

					return $oImg;
				}

				header('Content-Type: image/png');
				//header('Content-Disposition: attachment; filename="' . $this->sImagenNombre . '"');
				header('Content-Disposition: inline; filename="' . $this->sImagenNombre . '"');

				ob_start('output_handler');
				$bRet = imagepng($this->rImagen);
				ob_end_flush();

				imagedestroy($this->rImagen);
			}
			elseif($this->sDirectorio == 'base64'){ //salida de la imagen al navegador codificada en base64, y como objeto JSON con ['nombre':, 'datos':]
				header('Content-Type: application/json');

				ob_start();
				$bRet = imagepng($this->rImagen);
				$aJson = array('nombre' => $this->sImagenNombre, 'datos' => base64_encode(ob_get_contents()));
				ob_end_clean();

				echo(json_encode($aJson));

				imagedestroy($this->rImagen);
			}
			else{ //imagen guardada como archivo
				if(!is_writable($this->sDirectorio)){
					trigger_error('No tiene permisos de escritura en el directorio [' . $this->sDirectorio . ']', E_USER_ERROR);
					return false;
				}

				$bRet = imagepng($this->rImagen, $this->sDirectorio . $this->sImagenNombre);
				imagedestroy($this->rImagen);
			}
		}

		return $bRet;
	}

	/**
	 * Imitacion de la funcion "imageconvolution".
	 * Con un parametro menos, el primero, el recurso de imagen.
	 *
	 * @param string $filter
	 * @param string $filter_div
	 * @param integer $offset
	 * @return boolean
	 */
	protected function imageconvolution($filter, $filter_div, $offset){
		if(function_exists('imageconvolution')){
			return imageconvolution($this->rImagen, $filter, $filter_div, $offset);
		}

		if($this->rImagen == NULL) return 0;

		$srcback = imagecreatetruecolor($this->iAncho, $this->iAlto);
		$iColorFondo = imagecolorallocate($srcback, $this->aColorFondo[0], $this->aColorFondo[1], $this->aColorTexto[2]);
		imagealphablending($srcback, false);
		imagealphablending($this->rImagen, false);
		imagecopy($srcback, $this->rImagen, 0, 0, 0, 0, $this->iAncho, $this->iAlto);

		if($srcback==NULL) return 0;

		for($y=0;$y < $this->iAlto;++$y){
			for($x=0;$x < $this->iAncho;++$x){
				$new_r = $new_g = $new_b = 0;
				$alpha = imagecolorat($srcback, @$pxl[0], @$pxl[1]);
				$new_a = ($alpha >> 24);

				for($j=0;$j < 3;++$j){
					$yv = min(max($y - 1 + $j, 0), $this->iAlto - 1);
					for($i=0;$i < 3;++$i){
						$pxl = array(min(max($x - 1 + $i, 0), $this->iAncho - 1), $yv);
						$rgb = imagecolorat($srcback, $pxl[0], $pxl[1]);
						$new_r += (($rgb >> 16) & 0xFF) * $filter[$j][$i];
						$new_g += (($rgb >> 8) & 0xFF) * $filter[$j][$i];
						$new_b += ($rgb & 0xFF) * $filter[$j][$i];
						$new_a += ((0x7F000000 & $rgb) >> 24) * $filter[$j][$i];
					}
				}

				$new_r = ($new_r/$filter_div)+$offset;
				$new_g = ($new_g/$filter_div)+$offset;
				$new_b = ($new_b/$filter_div)+$offset;
				$new_a = ($new_a/$filter_div)+$offset;

				$new_r = ($new_r > 255)? 255 : (($new_r < 0)? 0:$new_r);
				$new_g = ($new_g > 255)? 255 : (($new_g < 0)? 0:$new_g);
				$new_b = ($new_b > 255)? 255 : (($new_b < 0)? 0:$new_b);
				$new_a = ($new_a > 127)? 127 : (($new_a < 0)? 0:$new_a);

				$new_pxl = imagecolorallocatealpha($this->rImagen, (int)$new_r, (int)$new_g, (int)$new_b, $new_a);
				if($new_pxl == -1) $new_pxl = imagecolorclosestalpha($this->rImagen, (int)$new_r, (int)$new_g, (int)$new_b, $new_a);
				if(($y >= 0) && ($y < $this->iAlto)) imagesetpixel($this->rImagen, $x, $y, $new_pxl);
			}
		}

		imagedestroy($srcback);

		return 1;
	}

	/**
	 * Filtrado de la imagen; emula la funcion "imagefilter" si no se encuentra (que este GD no implica que contenga esta funcion).
	 * Funcionalidades: negativo, escala de grises, control de brillo, de contraste, colorizado, deteccion de bordes, bajorrelieve, desenfoque gausiano, desenfoque, sketch, suavizado.
	 *
	 * Include this file whenever you have to use imageconvolution...
	 * you can use in your project, but keep the comment below :)
	 * great for any image manipulation library
	 * @author Chao Xu(Mgccl)
	 * @link www.webdevlogs.com
	 * @version 1.0 3/1/07
	 *
	 * @param integer $var Filtro a aplicar: IMG_FILTER_NEGATE, IMG_FILTER_GRAYSCALE, IMG_FILTER_BRIGHTNESS, IMG_FILTER_CONTRAST, IMG_FILTER_COLORIZE, IMG_FILTER_EDGEDETECT, IMG_FILTER_EMBOSS, IMG_FILTER_GAUSSIAN_BLUR, IMG_FILTER_SELECTIVE_BLUR, IMG_FILTER_MEAN_REMOVAL, IMG_FILTER_SMOOTH, IMG_FILTER_PIXELATE
	 * @param integer $arg1 Primer argumento del filtro para: IMG_FILTER_BRIGHTNESS, IMG_FILTER_CONTRAST, IMG_FILTER_COLORIZE, IMG_FILTER_SMOOTH, IMG_FILTER_PIXELATE
	 * @param integer $arg2 Segundo argumento del filtro para:  IMG_FILTER_COLORIZE, IMG_FILTER_PIXELATE
	 * @param integer $arg3 Tercer argumento del filtro para: IMG_FILTER_COLORIZE
	 * @param integer $arg4 Cuarto argumento del filtro para: IMG_FILTER_COLORIZE
	 * @return boolean
	 */
	protected function imagefilter($var, $arg1 = null, $arg2 = null, $arg3 = null, $arg4 = null){
		if(function_exists('imagefilter')){
			$aParams = array($this->rImagen, $var);
			if($arg1 !== null){
				$aParams[] = $arg1;
			}
			if($arg2 !== null){
				$aParams[] = $arg2;
			}
			if($arg3 !== null){
				$aParams[] = $arg3;
			}
			if($arg4 !== null){
				$aParams[] = $arg4;
			}

			return call_user_func_array('imagefilter', $aParams);
			//return imagefilter($this->rImagen, $var, $arg1, $arg2, $arg3, $arg4);
		}

		switch($var){
			case 0: //IMG_FILTER_NEGATE
				$y = 0;

				while($y < $this->iAlto){
					$x = 0;

					while($x < $this->iAncho){
						$rgb = imagecolorat($this->rImagen, $x, $y);
						$r = 255 - (($rgb >> 16) & 0xFF);
						$g = 255 - (($rgb >> 8) & 0xFF);
						$b = 255 - ($rgb & 0xFF);
						$a = $rgb >> 24;
						$new_pxl = imagecolorallocatealpha($this->rImagen, $r, $g, $b, $a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, $r, $g, $b, $a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);

						++$x;
					}

					++$y;
				}

				return true;
				break;
			case 1: //IMG_FILTER_GRAYSCALE
				$y = 0;

				while($y < $this->iAlto){
					$x = 0;

					while($x < $this->iAncho){
						$rgb = imagecolorat($this->rImagen, $x, $y);
						$a = $rgb >> 24;
						$r = ((($rgb >> 16) & 0xFF) * 0.299) + ((($rgb >> 8) & 0xFF) * 0.587) + (($rgb & 0xFF) * 0.114);
						$new_pxl = imagecolorallocatealpha($this->rImagen, $r, $r, $r, $a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, $r, $r, $r, $a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);

						++$x;
					}

					++$y;
				}

				return true;
				break;
			case 2: //IMG_FILTER_BRIGHTNESS
				$y = 0;

				while($y < $this->iAlto){
					$x = 0;

					while($x < $this->iAncho){
						$rgb = imagecolorat($this->rImagen, $x, $y);
						$r = (($rgb >> 16) & 0xFF) + $arg1;
						$g = (($rgb >> 8) & 0xFF) + $arg1;
						$b = ($rgb & 0xFF) + $arg1;
						$a = $rgb >> 24;
						$r = ($r > 255) ? 255 : (($r < 0) ? 0 : $r);
						$g = ($g > 255) ? 255 : (($g < 0) ? 0 : $g);
						$b = ($b > 255) ? 255 : (($b < 0) ? 0 : $b);
						$new_pxl = imagecolorallocatealpha($this->rImagen, $r, $g, $b, $a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, $r, $g, $b, $a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);

						++$x;
					}

					++$y;
				}

				return true;
				break;
			case 3: //IMG_FILTER_CONTRAST
				$contrast = pow((100 - $arg1) / 100, 2);
				$y = 0;

				while($y < $this->iAlto){
					$x = 0;

					while($x < $this->iAncho){
						$rgb = imagecolorat($this->rImagen, $x, $y);
						$a = $rgb >> 24;
						$r = (((((($rgb >> 16) & 0xFF) / 255) - 0.5) * $contrast) + 0.5) * 255;
						$g = (((((($rgb >> 8) & 0xFF) / 255) - 0.5) * $contrast) + 0.5) * 255;
						$b = ((((($rgb & 0xFF) / 255) - 0.5) * $contrast) + 0.5) * 255;
						$r = ($r > 255) ? 255 : (($r < 0) ? 0 : $r);
						$g = ($g > 255) ? 255 : (($g < 0) ? 0 : $g);
						$b = ($b > 255) ? 255 : (($b < 0) ? 0 : $b);
						$new_pxl = imagecolorallocatealpha($this->rImagen, $r, $g, $b, $a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, $r, $g, $b, $a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);

						++$x;
					}

					++$y;
				}

				return true;
				break;
			case 4: //IMG_FILTER_COLORIZE
				$x = 0;

				while($x < $this->iAncho){
					$y = 0;

					while($y < $this->iAlto){
						$rgb = imagecolorat($this->rImagen, $x, $y);
						$r = (($rgb >> 16) & 0xFF) + $arg1;
						$g = (($rgb >> 8) & 0xFF) + $arg2;
						$b = ($rgb & 0xFF) + $arg3;
						$a = $rgb >> 24;
						$r = ($r > 255) ? 255 : (($r < 0) ? 0 : $r);
						$g = ($g > 255) ? 255 : (($g < 0) ? 0 : $g);
						$b = ($b > 255) ? 255 : (($b < 0) ? 0 : $b);
						$new_pxl = imagecolorallocatealpha($this->rImagen, $r, $g, $b, $a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, $r, $g, $b, $a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);

						++$y;
					}

					++$x;
				}

				return true;
				break;
			case 5: //IMG_FILTER_EDGEDETECT
				return $this->imageconvolution(array(array(-1, 0, -1), array(0, 4, 0), array(-1, 0, -1)), 1, 127);
				break;
			case 6: //IMG_FILTER_EMBOSS
				return $this->imageconvolution(array(array(1.5, 0, 0), array(0, 0, 0), array(0, 0, -1.5)), 1, 127);
				break;
			case 7: //IMG_FILTER_GAUSSIAN_BLUR
				return $this->imageconvolution(array(array(1, 2, 1), array(2, 4, 2), array(1, 2, 1)), 16, 0);
				break;
			case 8: //IMG_FILTER_SELECTIVE_BLUR
				for($y = 0; $y < $this->iAlto; $y++){
					for($x = 0; $x < $this->iAncho; $x++){
						$flt_r_sum = $flt_g_sum = $flt_b_sum = 0;
						$cpxl = imagecolorat($this->rImagen, $x, $y);

						for($j = 0; $j < 3; $j++){
							for($i = 0; $i < 3; $i++){
								if(($j == 1) && ($i == 1)){
									$flt_r[1][1] = $flt_g[1][1] = $flt_b[1][1] = 0.5;
								}
								else{
									$pxl = imagecolorat($this->rImagen, $x - (3 >> 1) + $i, $y - (3 >> 1) + $j);

									$new_a = $pxl >> 24;
									//$r = (($pxl >> 16) & 0xFF);
									//$g = (($pxl >> 8) & 0xFF);
									//$b = ($pxl & 0xFF);
									$new_r = abs((($cpxl >> 16) & 0xFF) - (($pxl >> 16) & 0xFF));
									if($new_r != 0){
										$flt_r[$j][$i] = 1 / $new_r;
									}
									else{
										$flt_r[$j][$i] = 1;
									}

									$new_g = abs((($cpxl >> 8) & 0xFF) - (($pxl >> 8) & 0xFF));
									if($new_g != 0){
										$flt_g[$j][$i] = 1 / $new_g;
									}
									else{
										$flt_g[$j][$i] = 1;
									}

									$new_b = abs(($cpxl & 0xFF) - ($pxl & 0xFF));
									if($new_b != 0){
										$flt_b[$j][$i] = 1 / $new_b;
									}
									else{
										$flt_b[$j][$i] = 1;
									}
								}

								$flt_r_sum += $flt_r[$j][$i];
								$flt_g_sum += $flt_g[$j][$i];
								$flt_b_sum += $flt_b[$j][$i];
							}
						}

						for($j = 0; $j < 3; $j++){
							for($i = 0; $i < 3; $i++){
								if($flt_r_sum != 0){
									$flt_r[$j][$i] /= $flt_r_sum;
								}
								if($flt_g_sum != 0){
									$flt_g[$j][$i] /= $flt_g_sum;
								}
								if($flt_b_sum != 0){
									$flt_b[$j][$i] /= $flt_b_sum;
								}
							}
						}

						$new_r = $new_g = $new_b = 0;

						for($j = 0; $j < 3; $j++){
							for($i = 0; $i < 3; $i++){
								$pxl = imagecolorat($this->rImagen, $x - (3 >> 1) + $i, $y - (3 >> 1) + $j);
								$new_r += (($pxl >> 16) & 0xFF) * $flt_r[$j][$i];
								$new_g += (($pxl >> 8) & 0xFF) * $flt_g[$j][$i];
								$new_b += ($pxl & 0xFF) * $flt_b[$j][$i];
							}
						}

						$new_r = ($new_r > 255) ? 255 : (($new_r < 0) ? 0 : $new_r);
						$new_g = ($new_g > 255) ? 255 : (($new_g < 0) ? 0 : $new_g);
						$new_b = ($new_b > 255) ? 255 : (($new_b < 0) ? 0 : $new_b);
						$new_pxl = imagecolorallocatealpha($this->rImagen, (int) $new_r, (int) $new_g, (int) $new_b, $new_a);
						if($new_pxl == false){
							$new_pxl = imagecolorclosestalpha($this->rImagen, (int) $new_r, (int) $new_g, (int) $new_b, $new_a);
						}
						imagesetpixel($this->rImagen, $x, $y, $new_pxl);
					}
				}

				return true;
				break;
			case 9: //IMG_FILTER_MEAN_REMOVAL
				return $this->imageconvolution(array(array(-1, -1, -1),array(-1, 9, -1),array(-1, -1, -1)), 1, 0);
				break;
			case 10: //IMG_FILTER_SMOOTH
				return $this->imageconvolution(array(array(1, 1, 1),array(1, $arg1, 1),array(1, 1, 1)), $arg1 + 8, 0);
				break;
			case 11: //IMG_FILTER_PIXELATE
			default:
				trigger_error('imagefilter: tipo de filtro no soportado', E_USER_WARNING);
				return false;
		}

		return true;
	}

	/**
	 * Cambia colores selectivamente.
	 *
	 * @param integer $r Red
	 * @param integer $g Green
	 * @param integer $b Blue
	 * @return boolean
	 */
	protected function imagefilterhue($r, $g, $b){
		$rgb = $r + $g + $b;
		$col = array($r / $rgb, $b / $rgb, $g / $rgb);

		for($x = 0; $x < $this->iAlto; $x++){
			for($y = 0; $y < $this->iAncho; $y++){
				$rgb = imagecolorat($this->rImagen, $x, $y);
				$r = ($rgb >> 16) & 0xFF;
				$g = ($rgb >> 8) & 0xFF;
				$b = $rgb & 0xFF;
				$newR = $r * $col[0] + $g * $col[1] + $b * $col[2];
				$newG = $r * $col[2] + $g * $col[0] + $b * $col[1];
				$newB = $r * $col[1] + $g * $col[2] + $b * $col[0];
				imagesetpixel($this->rImagen, $x, $y,imagecolorallocate($this->rImagen, $newR, $newG, $newB));
			}
		}

		return true;
	}

	/**
	 * Crea un negativo de la imagen
	 *
	 * @return boolean
	 */
	/*protected function negativo(){
		if(function_exists('imagefilter')){
			return imagefilter($this->rImagen, IMG_FILTER_NEGATE);
		}

		for($x = 0; $x < $this->iAncho; ++$x){
			for($y = 0; $y < $this->iAlto; ++$y){
				$index = imagecolorat($this->rImagen, $x, $y);
				$rgb = imagecolorsforindex($index);
				$color = imagecolorallocate($this->rImagen, 255 - $rgb['red'], 255 - $rgb['green'], 255 - $rgb['blue']);

				imagesetpixel($this->rImagen, $x, $y, $color);
			}
		}

		return true;
	}*/

	/**
	 * Pone "ruido" en la imagen.
	 *
	 * @param integer $cant Cantidad de lineas a dibujar
	 * @return boolean
	 */
	protected function ruidoLineas($cant = 5){
		for($i = 0; $i < $cant; $i++){
			$color = imagecolorallocate($this->rImagen, rand(64, 192), rand(64, 192), rand(64, 192));
			if(rand(0, 1)){
				$r1 = rand(0, $this->iAncho);
				imageline($this->rImagen, $r1, 0, $r1 + rand(-5, 5), $this->iAlto, $color);
			}
			else{
				$r1 = rand(0, $this->iAlto);
				imageline($this->rImagen, 0, $r1, $this->iAncho, $r1 + rand(-5, 5), $color);
			}
		}

		return true;
	}

	/**
	 * Voltear de la imagen.
	 *
	 * @return boolean
	 */
	protected function voltear(){
		$rImagenAux = imagecreatetruecolor($this->iAncho, $this->iAlto);

		for($i = 0; $i < $this->iAncho; $i++){
			for($j = 0; $j < $this->iAlto; $j++){
				$ref = imagecolorat($this->rImagen, $i, $j);
				imagesetpixel($rImagenAux, $i, $this->iAlto - $j, $ref);
			}
		}

		imagedestroy($this->rImagen);
		$this->rImagen = &$rImagenAux;

		return true;
	}

}
