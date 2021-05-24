@REM Copyright (C) 2021 Stephen G. Tuggy

@REM https://github.com/vegastrike/Vega-Strike-Engine-Source

@REM This file is part of Vega Strike.

@REM Vega Strike is free software: you can redistribute it and/or modify
@REM it under the terms of the GNU General Public License as published by
@REM the Free Software Foundation, either version 3 of the License, or
@REM (at your option) any later version.

@REM Vega Strike is distributed in the hope that it will be useful,
@REM but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM GNU General Public License for more details.

@REM You should have received a copy of the GNU General Public License
@REM along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.

mkdir C:\Projects
pushd C:\Projects
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat -disableMetrics
set PATH=%PATH%;C:\Projects\vcpkg\downloads\tools\cmake-3.20.2-windows\cmake-3.20.2-windows-i386\bin
set VCPKG_DEFAULT_TRIPLET=x64-windows
set PYTHONHOME=C:\Projects\vcpkg\packages\python3_x64-windows\tools\python3
popd
