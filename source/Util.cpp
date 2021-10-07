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
//	filename = filename.replace("~", getpwuid(getuid())->pw_dir /*homedir*/);
	if (fileExists(filename))return filename;
	// todo: check wasm date, recompile if older than wasp / wast
	if (fileExists(filename + ".wasp"))return filename + ".wasp"s;
	if (fileExists(filename + ".wast"s))return filename + ".wast"s;
	if (fileExists(filename + ".wasm"))return filename + ".wasm"s;
	//		if (not filename.contains("/"))filename = findFile(project + "/" + filename) || filename;
	if (not filename.contains("/"))filename = findFile("lib/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("src/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("source/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("include/"s + filename) || filename;
	if (not filename.contains("/"))filename = findFile("samples/"s + filename) || filename;
	return fileExists(filename) ? filename : "";
}