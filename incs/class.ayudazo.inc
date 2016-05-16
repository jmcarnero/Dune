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
 * Multiples ayudantes en forma de metodos
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package ayudazo
 */
class Ayudazo {

	/**
	 * Constructor
	 */
	public function __construct(){
	}

	/**
	 * Destructor
	 */
	public function __destruct(){
		//parent::__destruct();
	}

	/**
	 * Recorta la cadena "$cad" a "$long" caracteres, sin cortar palabras
	 * si "$punto" la recorta aun mas, hasta el primer "$punto" (un solo caracter) desde el final de la cadena (para no cortar frases
	 *
	 * @todo excluir tags HTML "strip_tags($cad, '<a>')" deja "<a>"; controlar que no quede ninguno abierto despues de cortar la cadena (o se arrastra el tag a las demas noticias mostradas)
	 * @param string $cad Cadena a cortar
	 * @param integer $long Longitud de corte
	 * @param boolean $punto Si true corta en el punto mas proximo (previo) a la longitud de corte
	 * @return string
	 */
	public function cortaCadena($cad, $long, $punto = false){
		if(strlen($cad) <= $long) return(strip_tags($cad));
		$long -= 4; //se le resta ' ...';
		$cadCort = substr(strip_tags($cad), 0, $long);
		if($punto === false) $cadCort = substr($cadCort, 0, strrpos($cadCort, ' '));
		else $cadCort = substr($cadCort, 0, strrpos($cadCort, $punto));
		return($cadCort.' ...');
	}

	/*
	 * Two simple functions for encrypting and decrypting with RIJNDAEL 256
	//example
	echo RIJNDAEL_decrypt(RIJNDAEL_encrypt('Her name was lola!'));
	*/
	/*function RIJNDAEL_encrypt($text){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$key = "This is a very secret key";
		return base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $text, MCRYPT_MODE_ECB, $iv));
	}
	function RIJNDAEL_decrypt($text){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$key = "This is a very secret key";
		//I used trim to remove trailing spaces
		return trim(mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($text), MCRYPT_MODE_ECB, $iv));
	}*/

}
