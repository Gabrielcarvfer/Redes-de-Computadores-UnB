#! /usr/bin/env python3
# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
#
# Copyright (c) 2022 Gabriel Ferreira <gabrielcarvfer@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


import asyncio


async def waiting_routine(file):
    # Imagine we are parsing a huge file that takes a ton of time
    # then we return the contents
    asyncio.sleep(10)
    contents = "boop"
    return contents


async def main():
    # Trivial stuff
    # You call and wait for the response before continuing
    contents = await waiting_routine("large_file.txt")
    print(contents)

    # Not that trivial
    # You call, but don't wait for the response before continuing
    # A different thread reads the file while the program continues
    task = asyncio.create_task(waiting_routine("large_file.txt"))

    # program continues
    print("cont")
    print("cont")
    print("cont")
    print("cont")

    # Then, when we finally need the contents of the file, we wait for it
    contents2 = await task
    print(contents2)


# Async is different from normal programs
# There is a runner that runs a main routine
# and worker threads that wait for requests
# without ever blocking
if __name__ == "__main__":
    asyncio.run(main())
