//
// Created by rgnter on 25/11/2024.
//

#ifndef DATADIRECTOR_HPP
#define DATADIRECTOR_HPP

#include "Scheduler.hpp"
#include "server/Settings.hpp"

#include <cstdint>
#include <functional>
#include <future>
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
  //! A unique identifier.
  uint32_t uid{};
  //! A type identifier.
  uint32_t tid{};
  //! Amount of an item.
  uint32_t count{};
};

//! Character.
struct Character
{
  //!
  std::string name;
  //!
  std::string status;
  //!
  uint16_t level{};
  //!
  int32_t carrots{};
  //!
  int32_t cash{};

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

  //! TODO in db
  std::vector<Item> characterEquipment;
  //!
  std::vector<Item> horseEquipment;

  //!
  DatumUid mountUid{};
  //!
  DatumUid ranchUid{};

  //! TODO in db
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

//! User
struct User
{
  DatumUid characterUid;
  std::string name;
  std::string token;
};

} // namespace data

class DataDirector
{
public:
  template<typename T>
  struct View
  {
    explicit View(T& value)
      : _value(value)
    {
    }

    View(const View&) = delete;
    void operator=(const View&) = delete;

    View(View&& other)
      : _value(other._value)
      , _lock(std::move(other._lock))
    {
    }
    View& operator=(View&& other) noexcept
    {
      _value = other._value;
      _lock = std::move(other._lock);
      return *this;
    }

    bool IsAvailable()
    {
      return _value.available;
    }

    T& operator()()
    {
      return _value;
    }

  private:
    T& _value;
    std::unique_lock<std::mutex> _lock;
  };

  //!
  explicit DataDirector(
    Settings::DataSource settings = {});

  //! Establishes connection with the data source.
  void EstablishConnection();

  //! Neviem este
  std::future<data::User> GetUser(std::string const &name);

  //! Ani tu este neviem
  std::future<data::Character> GetCharacter(DatumUid uid);
private:
  template<typename T>
  struct Record
  {
    bool available{false};
    std::mutex mutex;
    T value;
  };

  //! username, promise
  std::unordered_map<std::string, std::promise<data::User>> _users;

  //! uid, promise
  std::unordered_map<DatumUid, std::promise<data::Character>> _characters;

  //!
  Settings::DataSource _settings;
  //!
  server::TaskLoop _taskLoop;

  //!
  std::mutex _connectionMtx{};
  //!
  std::unique_ptr<pqxx::connection> _connection{nullptr};
};

}

#endif //DATADIRECTOR_HPP