/*

OpenGL v1.1 loader for SDL.

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// OpenGL v1.1 loader for SDL.
//


#ifndef SDL_OGL11_LOADER_INCLUDED
#define SDL_OGL11_LOADER_INCLUDED


#include <string>


class SdlOgl11Loader
{
public:
	SdlOgl11Loader() = delete;


	//
	// Initializes the loader.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	// Notes:
	//    - Should be called after SDL initialization.
	//    - Call this before creating any OpenGL windows.
	//
	static bool initialize();

	//
	// Uninitializes the loader.
	//
	// Notes:
	//    - Should be called before SDL uninitialization.
	//    - Don't call any OpenGL function after this.
	//
	static void uninitialize();

	//
	// Gets a last error message.
	//
	// Returns:
	//    - A last error message.
	//
	static const std::string& get_error_message();


private:
	class Detail;
}; // SdlOgl11Loader


#endif // SDL_OGL11_LOADER_INCLUDED
