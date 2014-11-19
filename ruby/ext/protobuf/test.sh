#!/bin/bash

ruby extconf.rb
make && RUBYLIB=. ruby test.rb
