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

if(!defined('DUNE')) die('...');

/**
 * Plantilla base
 *
 * @author José M. Carnero
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="es" lang="es">
<head>
	<title><?php echo(empty($sTitle)?'':$sTitle); ?></title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="Language" content="es" />
<?php echo($sMetas); ?>
	<link href="<?php echo D_BASE_URL; ?>css/screen.css" rel="stylesheet" type="text/css" />
	<link href="<?php echo D_BASE_URL; ?>css/print.css" rel="stylesheet" media="print" type="text/css" />
<?php echo($sCss); ?>
	<script type="text/javascript" src="<?php echo D_BASE_URL; ?>scripts/JaSper_min.js"></script>
<?php echo $sJs; ?>
</head>
<body>
<?php echo $sContenidos; ?>
</body>
</html>
