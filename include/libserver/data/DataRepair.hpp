/**
 * Alicia Server - dedicated server software
 * Copyright (C) 2024 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef DATAREPAIR_HPP
#define DATAREPAIR_HPP

#include "libserver/data/DataDefinitions.hpp"

#include <chrono>

namespace server
{

class DataDirector;

namespace repair
{

//! The duration for which the retrieval of a datum has to keep failing before the datum
//! is considered damaged beyond repair. A retrieval may also fail for a transient reason,
//! such as the file being briefly inaccessible, so a datum is retrieved over and over
//! for the whole grace period before the references to it are dropped.
//! Has to stay well below the timeout of a character load.
constexpr std::chrono::seconds DamagedGracePeriod{2};

//! Drops the references a character holds to data which are missing or damaged
//! in the data source. Such data are produced when the server is shut down or crashes
//! before a newly created record is written, and without dropping the references
//! the character can never be loaded again.
//!
//! Data which have not been failing to retrieve for the whole `DamagedGracePeriod`
//! are queued for another retrieval attempt instead of being dropped,
//! so this needs to be called on every attempt of a character load.
//!
//! @param dataDirector Data director owning the character.
//! @param characterUid UID of the character to repair.
//! @returns `true` if any reference was dropped, `false` otherwise.
bool CleanseCharacterReferences(DataDirector& dataDirector, data::Uid characterUid);

//! Drops the references a user holds to data which are missing or damaged
//! in the data source.
//!
//! @param dataDirector Data director owning the user.
//! @param userName Name of the user to repair.
//! @returns `true` if any reference was dropped, `false` otherwise.
bool CleanseUserReferences(DataDirector& dataDirector, const std::string& userName);

} // namespace repair

} // namespace server

#endif // DATAREPAIR_HPP
