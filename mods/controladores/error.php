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
if(!defined('DUNE')) die('...');

/**
 * Controlador de errores
 *
 * @author José M. Carnero
 * @since 2014-11-26
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Error extends Controlazo {

	function __construct(){
		parent::__construct(__FILE__);

		$this->aDatos['aMod']['error'] = array('title' => $this->trad('Error'));

		$this->aDatos['sMetas'] = '<meta name="keywords" content="Sargazos.net">';
		$this->aDatos['sTitle'] = $this->aDatos['aMod']['error']['title'];
		$this->aDatos['sCss'] .= '';
		$this->aDatos['sJs'] .= '';
	}

}