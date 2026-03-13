#!/bin/sh

function mbuild_automake()
{
  mbuild_remake 3rdparty libmbedtls
  mbuild_remake 3rdparty libev 
}

function mbuild_autoclean()
{
  mbuild_clean 3rdparty libmbedtls
  mbuild_clean 3rdparty libev 
}