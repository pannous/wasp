//
// Created by me on 07.10.21.
//

#include "String.h"
#include "Util.h"
#include "unistd.h"

//bool fileExists(char* filename) {

bool fileExists(String filename) {
	log("checking "s + filename);
	if (filename.empty())return false;
	return access(filename.data, F_OK) == 0;
}

String findFile(String filename) {
	if (filename.empty())return "";
	//		const char *homedir = getpwuid(getuid())->pw_dir;
	//		filename = filename.replace("~",  homedir);
	if (fileExists(filename))return filename;
	if (fileExists(filename + ".wasm"))return filename + ".wasm"s;
	if (fileExists(filename + ".wast"s))return filename + ".wast"s;
	if (fileExists(filename + ".wasp"))return filename + ".wasp"s;
	//		if (not filename.contains("/"))filename = findFile(project + "/" + filename) || filename;
	if (not filename.contains("/"))filename = findFile("lib/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("src/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("source/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("include/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("samples/"s + filename) || filename;
//#if DEBUG
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	log(cwd);
	if (not fileExists(filename) and not filename.contains("/"))
		if (contains(cwd, "cmake-build"))
			return findFile("../"s + filename);// todo DANGER don't use outside cmake-build-default tests!
//#endif
	return fileExists(filename) ? filename : "";
}