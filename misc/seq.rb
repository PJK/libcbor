#!/usr/bin/env ruby
# SPDX-FileCopyrightText: 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT

Integer($*[0]).times {|i| print "0x%02X, " % ((Integer($*[1]) + i) % 0xFF) }
