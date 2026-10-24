#!/usr/bin/env ruby
# SPDX-FileCopyrightText: 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT

(Integer($*[0])..Integer($*[1])).each {|i| puts "case 0x%02X:" % i}
