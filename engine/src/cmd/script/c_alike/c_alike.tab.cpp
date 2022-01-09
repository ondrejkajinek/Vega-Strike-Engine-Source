/**
* c_alike.tab.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include <string>
#include <stdio.h>
using std::string;
bool have_yy_error=false;
string parseCalike(char const *filename)
{
	return "";
}

int yylineno=0;
int yyerror(char *s){
  printf("(yy)error:  line %d text --\n",yylineno+1);

  return 1;
}

int yywrap(){
	return 1;
}


int yylex() {
	return 1;
}
