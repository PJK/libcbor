#!/usr/bin/env ruby
# SPDX-FileCopyrightText: 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT

lst = (ARGV.empty? ? STDIN.read : IO.binread(ARGV[0])).bytes.map {|_| '0x%02X' % _ }
puts lst.size
puts lst.join(', ')
