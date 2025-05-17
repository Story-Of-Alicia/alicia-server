//
// Created by rgnter on 14/05/2025.
//

#ifndef FILEDATASOURCE_HPP
#define FILEDATASOURCE_HPP

#include <libserver/data/DataDefinitions.hpp>
#include <libserver/data/DataSource.hpp>


namespace soa
{

class FileDataSource
  : public DataSource
{
public:
  void Initialize(const std::filesystem::path& path);
  void Terminate();

  void RetrieveUser(std::string name, data::User& user) override;
  void StoreUser(std::string name, const data::User& user) override;
  void RetrieveCharacter(data::Uid uid, data::Character& character) override;
  void StoreCharacter(data::Uid uid, const data::Character& character) override;
  void RetrieveItem(data::Uid uid, data::Item& item) override;
  void StoreItem(data::Uid uid, const data::Item& item) override;
  void RetrieveHorse(data::Uid uid, data::Horse& horse) override;
  void StoreHorse(data::Uid uid, const data::Horse& horse) override;
  void RetrieveRanch(data::Uid uid, data::Ranch& ranch) override;
  void StoreRanch(data::Uid uid, const data::Ranch& ranch) override;

private:
  std::filesystem::path _path;
  std::filesystem::path _usersPath;
  std::filesystem::path _charactersPath;
  std::filesystem::path _metaFilePath;

  uint32_t _sequentialUid = 0;
};

}

#endif //FILEDATASOURCE_HPP
