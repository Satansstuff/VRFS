#pragma once

#include "filesystem.h"

class ShellFunctions
{

	FileSystem *f;
public:
	ShellFunctions();
	~ShellFunctions();
	void format();

	void createImage(const std::string& filepath);

	void restoreImage(const std::string& filepath);

	std::string create(const std::string& filepath);

	std::string cat(const std::string& filepath);

	std::string ls(const std::string& path);

	void cp(const std::string& oldfile, const std::string& newfile);

	std::string mkdir(const std::string& dirpath);

	std::string cd(const std::string& dirpath);

	std::string pwd();

	std::string rm(const std::string& filepath);

	void append(const std::string& filepath1, const std::string& filepath2);

	std::string mv(const std::string& sourcepath, const std::string& destpath);

	std::string chmod(const std::string &file, bool r, bool w);

	std::string remove(const std::string &file);

	std::string currentDirName();
};

