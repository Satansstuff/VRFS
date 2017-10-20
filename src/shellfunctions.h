#pragma once

#include "filesystem.h"

class ShellFunctions
{

	FileSystem fs;
public:

	void format();

	void createImage(const std::string& filepath);

	void restoreImage(const std::string& filepath);

	void create(const std::string& filepath);

	void cat(const std::string& filepath);

	void ls(const std::string& path);

	void cp(const std::string& oldfile, const std::string& newfile);

	void mkdir(const std::string& dirpath);

	void cd(const std::string& dirpath);

	void pwd();

	void rm(const std::string& filepath);

	void append(const std::string& filepath1, const std::string& filepath2);

	void mv(const std::string& sourcepath, const std::string& destpath);

	void chmod();
};

