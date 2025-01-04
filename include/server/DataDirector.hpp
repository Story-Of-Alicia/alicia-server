//
// Created by rgnter on 25/11/2024.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "server/Settings.hpp"

#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

#include <pqxx/pqxx>

namespace alicia
{

//! Unique datum identifier.
using DatumUid = uint32_t;
//! Invalid datum identifier.
constexpr DatumUid InvalidDatumUid = 0;

namespace data
{

//! Item.
struct Item
{
  //!
  uint32_t uid{};
  //!
  uint32_t tid{};
  //!
  uint32_t count{};
};

//! Character.
struct Character
{
  //!
  std::string nickName;
  //!
  uint16_t level{};
  //!
  int32_t carrots{};
  //!
  std::string status;

  //!
  enum class AgeGroup
  {
    Kid,
    Teenager,
    Highschooler,
    Adult
  } ageGroup = AgeGroup::Kid;

  //!
  enum class Gender
  {
    Unspecified,
    Boy,
    Girl
  } gender = Gender::Unspecified;

  //!
  struct Appearance
  {
    //!
    uint8_t charId{};
    //! FaceId
    uint8_t mouthSerialId{};
    //! EyeId
    uint8_t faceSerialId{};
    //! FigFace
    uint16_t headVolume{};
    //! FigTall
    uint16_t height{};
    //! FigVolume
    uint16_t thighVolume{};
    //! FigShape
    uint16_t legVolume{};
  } appearance{};

  //!
  std::vector<Item> characterEquipment;
  //!
  std::vector<Item> horseEquipment;

  //!
  DatumUid mountUid{};
  //!
  DatumUid ranchUid{};

  //!
  std::vector<DatumUid> horses{};
};

//! Horsie.
struct Horse
{
  //!
  uint32_t uid{};
  //!
  uint32_t tid{};
  //! Max length is 255.
  std::string name{};

  //!
  struct Appearance
  {
    //!
    uint8_t skinId{};
    //!
    uint8_t maneId{};
    //!
    uint8_t tailId{};
    //!
    uint8_t faceId{};
    //!
    uint8_t scale{};
    //!
    uint8_t legLength{};
    //!
    uint8_t legVolume{};
    //!
    uint8_t bodyLength{};
    //!
    uint8_t bodyVolume{};
  } appearance{};

  //!
  struct Stats
  {
    //!
    uint32_t agility{};
    //!
    uint32_t spirit{};
    //!
    uint32_t speed{};
    //!
    uint32_t strength{};
    //!
    uint32_t ambition{};
  } stats{};

  //!
  uint32_t rating{};
  //!
  uint8_t clazz{};
  //!
  uint8_t grade{};
  //!
  uint16_t growthPoints{};

  struct Mastery
  {
    uint32_t magic{};
    uint32_t jumping{};
    uint32_t sliding{};
    //! Divided by 10?
    uint32_t gliding{};
  } mastery{};
};

//! Ranch
struct Ranch
{
  std::string ranchName;
};

//! User.
struct User
{
  std::string _token;

  DatumUid characterUid;
};


}

//! Datum consumer is a callback which provides access to a value,
//! to which access is guaranteed while in the consumer function.
template<typename T>
using DatumConsumer = std::function<void(T value)>;

class DataDirector
{
public:
  //!
  template<typename Val>
  struct Datum
  {
    Val value;
    std::mutex lock;
  };

  template<typename Val>
  struct DatumAccess
  {
    //!
    explicit DatumAccess(Datum<Val>& datum)
      : _datum(datum)
      , _accessLock(_datum.lock)
    {
    }

    //! Deleted copy constructor.
    DatumAccess(const DatumAccess&) noexcept = delete;
    //! Move constructor.
    DatumAccess(DatumAccess&& other) noexcept
      : _datum(other._datum)
    {
      other._accessLock.unlock();
      other._accessLock.release();

      _accessLock = _datum.lock;
    }

    ~DatumAccess()
    {
      _accessLock.unlock();
    }

    [[nodiscard]] Val* operator->() const noexcept {
      return &_datum.value;
    }

  private:
    Datum<Val>& _datum;
    std::unique_lock<std::mutex> _accessLock;
  };

  explicit DataDirector(
    Settings::DataSource settings = {});

  //!
  DatumAccess<data::User::Token> GetToken(
    const std::string& name);

  //!
  DatumAccess<data::User> GetUser(
    const std::string& name);
  //!
  DatumAccess<data::Character> GetCharacter(
    DatumUid characterUid);
  //!
  DatumAccess<data::Horse> GetHorse(
    DatumUid mountUid);
  //!
  DatumAccess<data::Ranch> GetRanch(
    DatumUid ranchUid);

private:
  //!
  std::unordered_map<std::string, Datum<data::User>> _users;
  //!
  std::unordered_map<DatumUid, Datum<data::Character>> _characters;
  //!
  std::unordered_map<DatumUid, Datum<data::Horse>> _horses;
  //!
  std::unordered_map<DatumUid, Datum<data::Ranch>> _ranches;

  //!
  Settings::DataSource _settings;

  //!
  std::mutex _connectionMtx;
  //!
  pqxx::connection _connection;
};

}

#endif //DATADIRECTOR_HPP