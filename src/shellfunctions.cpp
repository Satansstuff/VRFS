#include <string>
#include "shellfunctions.h"
#include "filesystem.h"

ShellFunctions::ShellFunctions()
{
	f = new FileSystem();
}
ShellFunctions::~ShellFunctions()
{
	delete f;
}
std::string ShellFunctions::remove(const std::string &file)
{
	return std::to_string(f->remove(file));
}
void ShellFunctions::format()
{
	delete f;
	f = new FileSystem();
}
std::string ShellFunctions::pwd()
{
	return f->getCurrentDirectory();
}
std::string ShellFunctions::createImage(const std::string& filepath)
{
	return std::string("derp");
}
std::string ShellFunctions::restoreImage(const std::string& filepath)
{
	return std::string("derp");
}
std::string ShellFunctions::create(const std::string& filepath)
{
	return std::to_string((int)f->create(filepath));
}
std::string ShellFunctions::cat(const std::string& filepath)
{
	File toRead = f->open(filepath);
	std::string data = f->read(toRead);
	return data;
}
std::string ShellFunctions::ls(const std::string& path)
{
	return f->ls(path);
}
void ShellFunctions::cp(const std::string& oldfile, const std::string& newfile)
{
	f->create(newfile);
	File f1 = f->open(newfile);
	File f2 = f->open(oldfile);
	std::string data = f->read(f2);
	f->write(f1, data);
	f->close(f1);
	f->close(f2);
}
std::string ShellFunctions::mkdir(const std::string& dirpath)
{
	return std::to_string((int)f->mkdir(dirpath));
}
std::string ShellFunctions::cd(const std::string& dirpath)
{
	return std::to_string((int)f->cd(dirpath));
}
std::string ShellFunctions::rm(const std::string& filepath)
{
	return std::to_string(f->remove(filepath));
}
std::string ShellFunctions::append(const std::string& filepath1, const std::string& filepath2)
{
	return std::string("derp");
}
std::string ShellFunctions::mv(const std::string& sourcepath, const std::string& destpath)
{
	std::string s;
	File file = f->open(sourcepath);
	if(!file)
	{
		return std::string("Failed");
	}
	s = f->read(file);
	f->create(destpath);
	File newfile = f->open(destpath);
	if(!newfile)
	{
		return std::string("Failed");
	}
	f->write(newfile,s);
	f->close(file);
	f->remove(sourcepath);
	f->close(newfile);
	return std::string("");
}
std::string ShellFunctions::chmod(const std::string &file, bool r, bool w)
{
	return std::to_string(f->chmod(file,r,w));
}