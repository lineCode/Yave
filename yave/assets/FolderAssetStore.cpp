/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/


#include "FolderAssetStore.h"

#include <y/io/File.h>
#include <y/io/BuffWriter.h>
#include <y/io/BuffReader.h>

#include <atomic>

#ifndef YAVE_NO_STDFS

namespace yave {

FolderAssetStore::FolderFileSystemModel::FolderFileSystemModel(std::string_view root) : _root(root) {
}

const core::String& FolderAssetStore::FolderFileSystemModel::root_path() const {
	return _root;
}

FileSystemModel::Result<core::String> FolderAssetStore::FolderFileSystemModel::current_path() const  {
	return core::Ok(core::String());
}

FileSystemModel::Result<core::String> FolderAssetStore::FolderFileSystemModel::parent_path(std::string_view path) const  {
	if(!path.empty()) {
		if(is_delimiter(path[path.size() - 1])) {
			path = path.substr(0, path.size() - 1);
		}

		for(usize i = 1; i != path.size(); ++i) {
			usize index = path.size() - i;
			if(is_delimiter(path[index - 1])) {
				return core::Ok(core::String(path.substr(0, index)));
			}
		}
	}
	return core::Err();
}

FileSystemModel::Result<bool> FolderAssetStore::FolderFileSystemModel::exists(std::string_view path) const  {
	return LocalFileSystemModel::exists(join(_root, path));
}

FileSystemModel::Result<bool> FolderAssetStore::FolderFileSystemModel::is_directory(std::string_view path) const  {
	return LocalFileSystemModel::is_directory(join(_root, path));
}

FileSystemModel::Result<core::String> FolderAssetStore::FolderFileSystemModel::absolute(std::string_view path) const  {
	return core::Ok(core::String(path));
}

FileSystemModel::Result<> FolderAssetStore::FolderFileSystemModel::for_each(std::string_view path, const for_each_f& func) const  {
	return LocalFileSystemModel::for_each(join(_root, path), func);
}

FileSystemModel::Result<> FolderAssetStore::FolderFileSystemModel::create_directory(std::string_view path) const  {
	return LocalFileSystemModel::create_directory(join(_root, path));
}

FileSystemModel::Result<> FolderAssetStore::FolderFileSystemModel::remove(std::string_view path) const  {
	return LocalFileSystemModel::remove(join(_root, path));
}

FileSystemModel::Result<> FolderAssetStore::FolderFileSystemModel::rename(std::string_view from, std::string_view to) const  {
	return LocalFileSystemModel::rename(join(_root, from), join(_root, to));
}



FolderAssetStore::FolderAssetStore(std::string_view path) :
		_filesystem(path),
		_index_file_path(_filesystem.join(_filesystem.root_path(), ".index")) {

	log_msg("Store index file: " + _index_file_path);
	if(!_filesystem.create_directory(".")) {
		log_msg("Unable to create store directory.", Log::Error);
	}
	if(!read_index()) {
		log_msg("Unable to read index.", Log::Error);
	}
}

FolderAssetStore::~FolderAssetStore() {
	if(!write_index()) {
		log_msg("Unable to write index.", Log::Error);
	}
}

const FileSystemModel* FolderAssetStore::filesystem() const {
	return &_filesystem;
}

AssetStore::Result<> FolderAssetStore::read_index() {
	std::unique_lock lock(_lock);
	y_defer(y_debug_assert(_from_id.size() == _from_name.size()));

	try {
		auto file = io::BuffReader(std::move(io::File::open(_index_file_path).or_throw("Unable to open file.")));
		_from_id.clear();
		_from_name.clear();
		_id_factory.deserialize(file);
		while(!file.at_end()) {
			auto entry = std::make_unique<Entry>(serde::deserialized<Entry>(file));
			_from_id[entry->id] = entry.get();
			_from_name[entry->name] = std::move(entry);
		}
	} catch(std::exception& e) {
		log_msg(fmt("Exception while reading index file: %", e.what()), Log::Error);
		log_msg(fmt("% assets imported", _from_id.size()), Log::Error);
		return core::Err(ErrorType::FilesytemError);
	}

	return core::Ok();
}

AssetStore::Result<> FolderAssetStore::write_index() const {
	std::unique_lock lock(_lock);
	y_defer(y_debug_assert(_from_id.size() == _from_name.size()));

	try {
		if(auto r = io::File::create(_index_file_path)) {
			auto file = io::BuffWriter(std::move(r.unwrap()));
			_id_factory.serialize(file);
			for(const auto& row : _from_id) {
				const Entry& entry = *row.second;
				entry.serialize(file);
			}
		}
	} catch(std::exception& e) {
		log_msg(fmt("Exception while writing index file: %", e.what()), Log::Error);
		return core::Err(ErrorType::FilesytemError);
	}
	return core::Ok();
}

AssetStore::Result<AssetId> FolderAssetStore::import(io::ReaderRef data, std::string_view dst_name) {
	std::unique_lock lock(_lock);

	{
		auto dst_dir = _filesystem.parent_path(dst_name);
		if(!dst_dir) {
			return core::Err(ErrorType::FilesytemError);
		}
		if(!_filesystem.exists(dst_dir.unwrap()).unwrap_or(false) &&
		   !_filesystem.create_directory(dst_dir.unwrap())) {
			return core::Err(ErrorType::FilesytemError);
		}
	}

	auto& entry = _from_name[dst_name];
	if(entry) {
		return core::Err(ErrorType::AlreadyExistingID);
	}

	// remove/optimize
	y_defer(write_index().ignore());
	y_defer(y_debug_assert(_from_id.size() == _from_name.size()));

	auto dst_file = _filesystem.join(_filesystem.root_path(), dst_name);
	if(!io::File::copy(data, dst_file)) {
		_from_name.erase(_from_name.find(dst_name));
		return core::Err(ErrorType::FilesytemError);
	}

	AssetId id = _id_factory.create_id();
	entry = std::make_unique<Entry>(Entry{dst_name, id});
	_from_id[id] = entry.get();
	return core::Ok(id);
}

AssetStore::Result<> FolderAssetStore::replace(io::ReaderRef data, AssetId id) {
	std::unique_lock lock(_lock);

	auto& entry = _from_id[id];
	if(!entry) {
		return core::Err(ErrorType::UnknownID);
	}

	// remove/optimize
	y_defer(write_index().ignore());
	y_defer(y_debug_assert(_from_id.size() == _from_name.size()));

	auto dst_file = _filesystem.join(_filesystem.root_path(), entry->name);
	if(!io::File::copy(data, dst_file)) {
		return core::Err(ErrorType::FilesytemError);
	}
	return core::Ok();
}

AssetStore::Result<AssetId> FolderAssetStore::id(std::string_view name) const {
	y_profile();
	std::unique_lock lock(_lock);

	y_debug_assert(_from_id.size() == _from_name.size());
	if(auto it = _from_name.find(name); it != _from_name.end()) {
		return core::Ok(it->second->id);
	}
	return core::Err(ErrorType::UnknownID);
}

AssetStore::Result<io::ReaderRef> FolderAssetStore::data(AssetId id) const {
	y_profile();
	std::unique_lock lock(_lock);

	y_debug_assert(_from_id.size() == _from_name.size());
	if(auto it = _from_id.find(id); it != _from_id.end()) {
		auto filename = _filesystem.join(_filesystem.root_path(), it->second->name);
		if(auto file = io::File::open(filename)) {
			return core::Ok(io::ReaderRef(std::move(file.unwrap())));
		}
		return core::Err(ErrorType::FilesytemError);
	}
	return core::Err(ErrorType::UnknownID);
}


AssetStore::Result<> FolderAssetStore::remove(AssetId id) {
	std::unique_lock lock(_lock);

	auto it = _from_id.find(id);
	if(it == _from_id.end()) {
		return core::Err(ErrorType::UnknownID);
	}

	const auto& name = it->second->name;
	if(!filesystem()->remove(name)) {
		return core::Err(ErrorType::FilesytemError);
	}

	_from_name.erase(_from_name.find(name));
	_from_id.erase(it);

	y_debug_assert(_from_id.size() == _from_name.size());
	return write_index();
}

AssetStore::Result<> FolderAssetStore::rename(AssetId id, std::string_view new_name) {
	if(new_name.empty()) {
		return core::Err(ErrorType::InvalidName);
	}

	std::unique_lock lock(_lock);

	auto it = _from_id.find(id);
	if(it == _from_id.end()) {
		return core::Err(ErrorType::UnknownID);
	}

	if(_from_name.find(new_name) != _from_name.end()) {
		return core::Err(ErrorType::UnknownID);
	}

	const auto& old_name = it->second->name;
	if(!filesystem()->rename(old_name, new_name)) {
		return core::Err(ErrorType::AlreadyExistingID);
	}

	auto entry_it = _from_name.find(old_name);
	std::unique_ptr<Entry> entry = std::move(entry_it->second);
	entry->name = new_name;
	_from_name.erase(entry_it);
	_from_name[new_name] = std::move(entry);

	y_debug_assert(_from_id.size() == _from_name.size());
	return write_index();
}

AssetStore::Result<> FolderAssetStore::remove(std::string_view name) {
	{
		std::unique_lock lock(_lock);
		if(filesystem()->is_directory(name).unwrap_or(false)) {
			if(!filesystem()->remove(name)) {
				return core::Err(ErrorType::FilesytemError);
			}

			core::Vector<decltype(_from_name)::iterator> to_remove;
			for(auto it = _from_name.begin(); it != _from_name.end(); ++it) {
				if(it->first.starts_with(name)) {
					to_remove << it;
				}
			}

			for(const auto& it : to_remove) {
				_from_id.erase(it->second->id);
				_from_name.erase(it);
			}

			y_debug_assert(_from_id.size() == _from_name.size());
			return write_index();
		}
	}

	if(auto i = id(name)) {
		return remove(i.unwrap());
	}
	return core::Err(ErrorType::UnknownID);
}

AssetStore::Result<> FolderAssetStore::rename(std::string_view from, std::string_view to) {
	if(to.empty()) {
		return core::Err(ErrorType::InvalidName);
	}

	{
		std::unique_lock lock(_lock);
		if(filesystem()->is_directory(from).unwrap_or(false)) {
			if(!filesystem()->rename(from, to)) {
				return core::Err(ErrorType::FilesytemError);
			}

			decltype(_from_name) from_name;
			from_name.reserve(_from_name.size());
			for(auto& asset : _from_name) {
				core::String& name = asset.second->name;
				if(name.starts_with(from)) {
					name = fmt("%%", to, name.sub_str(from.size()));
				}
				from_name.emplace(std::make_pair(name, std::move(asset.second)));
			}
			std::swap(_from_name, from_name);
			y_debug_assert(_from_id.size() == _from_name.size());
			return write_index();
		}
	}

	if(auto f = id(from)) {
		return rename(f.unwrap(), to);
	}
	return core::Err(ErrorType::UnknownID);
}

AssetStore::Result<> FolderAssetStore::write(AssetId id, io::ReaderRef data) {
	std::unique_lock lock(_lock);

	y_debug_assert(_from_id.size() == _from_name.size());
	if(auto it = _from_id.find(id); it != _from_id.end()) {
		auto filename = _filesystem.join(_filesystem.root_path(), it->second->name);
		if(io::File::copy(data, filename)) {
			return core::Ok();
		}
		return core::Err(ErrorType::FilesytemError);
	}
	return core::Err(ErrorType::UnknownID);
}

void FolderAssetStore::clean_index() {
	std::unique_lock lock(_lock);

	core::Vector<decltype(_from_name)::iterator> to_remove;
	for(auto it = _from_name.begin(); it != _from_name.end(); ++it) {
		if(!filesystem()->exists(it->first).unwrap_or(true)) {
			to_remove << it;
		}
	}

	for(const auto& it : to_remove) {
		log_msg(fmt("\"%\" erased.", it->first));
		_from_id.erase(it->second->id);
		_from_name.erase(it);
	}

	y_debug_assert(_from_id.size() == _from_name.size());
	write_index().ignore();
	log_msg("Index cleaned.");
}

}

#endif // YAVE_NO_STDFS
