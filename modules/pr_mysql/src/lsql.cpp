#include "lsql.hpp"
#include "mysql_connection.hpp"
#include "mysql_statement.hpp"
#include "sqlite_connection.hpp"
#include "sqlite_statement.hpp"
#include "lsql_connection.hpp"
#include "lsql_statement.hpp"
#include "lsql_blob.hpp"
#include "lsql_value.hpp"
#include <luainterface.hpp>
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/functioncallback.h>
#include <ldatastream.h>
#include <ishared.hpp>
#include <sqlite3.h>
#include <iostream>

void Lua::sql::register_lua_library(Lua::Interface &l)
{
	Lua::RegisterLibrary(l.GetState(),"sql",{
		{"connect",Lua::sql::open},
		{"create_value",Lua::sql::create_value}
		//{"result_to_string",Lua::sql::result_to_string}
	});

	Lua::RegisterLibraryEnums(l.GetState(),"sql",{
		{"SQLITE_OPEN_READONLY",SQLITE_OPEN_READONLY},
		{"SQLITE_OPEN_READWRITE",SQLITE_OPEN_READWRITE},
		{"SQLITE_OPEN_CREATE",SQLITE_OPEN_CREATE},
		{"SQLITE_OPEN_DELETEONCLOSE",SQLITE_OPEN_DELETEONCLOSE},
		{"SQLITE_OPEN_EXCLUSIVE",SQLITE_OPEN_EXCLUSIVE},
		{"SQLITE_OPEN_AUTOPROXY",SQLITE_OPEN_AUTOPROXY},
		{"SQLITE_OPEN_URI",SQLITE_OPEN_URI},
		{"SQLITE_OPEN_MEMORY",SQLITE_OPEN_MEMORY},
		{"SQLITE_OPEN_MAIN_DB",SQLITE_OPEN_MAIN_DB},
		{"SQLITE_OPEN_TEMP_DB",SQLITE_OPEN_TEMP_DB},
		{"SQLITE_OPEN_TRANSIENT_DB",SQLITE_OPEN_TRANSIENT_DB},
		{"SQLITE_OPEN_MAIN_JOURNAL",SQLITE_OPEN_MAIN_JOURNAL},
		{"SQLITE_OPEN_TEMP_JOURNAL",SQLITE_OPEN_TEMP_JOURNAL},
		{"SQLITE_OPEN_SUBJOURNAL",SQLITE_OPEN_SUBJOURNAL},
		{"SQLITE_OPEN_MASTER_JOURNAL",SQLITE_OPEN_MASTER_JOURNAL},
		{"SQLITE_OPEN_NOMUTEX",SQLITE_OPEN_NOMUTEX},
		{"SQLITE_OPEN_FULLMUTEX",SQLITE_OPEN_FULLMUTEX},
		{"SQLITE_OPEN_SHAREDCACHE",SQLITE_OPEN_SHAREDCACHE},
		{"SQLITE_OPEN_PRIVATECACHE",SQLITE_OPEN_PRIVATECACHE},
		{"SQLITE_OPEN_WAL",SQLITE_OPEN_WAL},

		{"TYPE_UNKNOWN",umath::to_integral(::sql::Type::Unknown)},
		{"TYPE_BIT",umath::to_integral(::sql::Type::Bit)},
		{"TYPE_TINY_INT",umath::to_integral(::sql::Type::TinyInt)},
		{"TYPE_SMALL_INT",umath::to_integral(::sql::Type::SmallInt)},
		{"TYPE_MEDIUM_INT",umath::to_integral(::sql::Type::MediumInt)},
		{"TYPE_INTEGER",umath::to_integral(::sql::Type::Integer)},
		{"TYPE_BIG_INT",umath::to_integral(::sql::Type::BigInt)},
		{"TYPE_REAL",umath::to_integral(::sql::Type::Real)},
		{"TYPE_DOUBLE",umath::to_integral(::sql::Type::Double)},
		{"TYPE_DECIMAL",umath::to_integral(::sql::Type::Decimal)},
		{"TYPE_NUMERIC",umath::to_integral(::sql::Type::Numeric)},
		{"TYPE_CHAR",umath::to_integral(::sql::Type::Char)},
		{"TYPE_BINARY",umath::to_integral(::sql::Type::Binary)},
		{"TYPE_VARCHAR",umath::to_integral(::sql::Type::VarChar)},
		{"TYPE_VARBINARY",umath::to_integral(::sql::Type::VarBinary)},
		{"TYPE_LONG_VARCHAR",umath::to_integral(::sql::Type::LongVarChar)},
		{"TYPE_LONG_VARBINARY",umath::to_integral(::sql::Type::LongVarBinary)},
		{"TYPE_TIMESTAMP",umath::to_integral(::sql::Type::TimeStamp)},
		{"TYPE_DATE",umath::to_integral(::sql::Type::Date)},
		{"TYPE_TIME",umath::to_integral(::sql::Type::Time)},
		{"TYPE_YEAR",umath::to_integral(::sql::Type::Year)},
		{"TYPE_GEOMETRY",umath::to_integral(::sql::Type::Geometry)},
		{"TYPE_ENUM",umath::to_integral(::sql::Type::Enum)},
		{"TYPE_SET",umath::to_integral(::sql::Type::Set)},
		{"TYPE_NULL",umath::to_integral(::sql::Type::Null)},
		{"TYPE_INITIAL",umath::to_integral(::sql::Type::Initial)},

		{"CONNECTION_TYPE_SQLITE",umath::to_integral(::sql::ConnectionType::SQLite)},
		{"CONNECTION_TYPE_MYSQL",umath::to_integral(::sql::ConnectionType::MySQL)},

		{"RESULT_OK",umath::to_integral(::sql::Result::Ok)},
		{"RESULT_ERROR",umath::to_integral(::sql::Result::Error)},
		{"RESULT_INTERNAL",umath::to_integral(::sql::Result::Internal)},
		{"RESULT_PERM",umath::to_integral(::sql::Result::Perm)},
		{"RESULT_ABORT",umath::to_integral(::sql::Result::Abort)},
		{"RESULT_BUSY",umath::to_integral(::sql::Result::Busy)},
		{"RESULT_LOCKED",umath::to_integral(::sql::Result::Locked)},
		{"RESULT_NO_MEM",umath::to_integral(::sql::Result::NoMem)},
		{"RESULT_READ_ONLY",umath::to_integral(::sql::Result::ReadOnly)},
		{"RESULT_INTERRUPT",umath::to_integral(::sql::Result::Interrupt)},
		{"RESULT_IO_ERR",umath::to_integral(::sql::Result::IoErr)},
		{"RESULT_CORRUPT",umath::to_integral(::sql::Result::Corrupt)},
		{"RESULT_NOT_FOUND",umath::to_integral(::sql::Result::NotFound)},
		{"RESULT_FULL",umath::to_integral(::sql::Result::Full)},
		{"RESULT_CANT_OPEN",umath::to_integral(::sql::Result::CantOpen)},
		{"RESULT_PROTOCOL",umath::to_integral(::sql::Result::Protocol)},
		{"RESULT_EMPTY",umath::to_integral(::sql::Result::Empty)},
		{"RESULT_SCHEMA",umath::to_integral(::sql::Result::Schema)},
		{"RESULT_TOO_BIG",umath::to_integral(::sql::Result::TooBig)},
		{"RESULT_CONSTRAINT",umath::to_integral(::sql::Result::Constraint)},
		{"RESULT_MISMATCH",umath::to_integral(::sql::Result::Mismatch)},
		{"RESULT_MISUSE",umath::to_integral(::sql::Result::Misuse)},
		{"RESULT_NO_LFS",umath::to_integral(::sql::Result::Nolfs)},
		{"RESULT_AUTH",umath::to_integral(::sql::Result::Auth)},
		{"RESULT_FORMAT",umath::to_integral(::sql::Result::Format)},
		{"RESULT_RANGE",umath::to_integral(::sql::Result::Range)},
		{"RESULT_NOT_A_DB",umath::to_integral(::sql::Result::NotADb)},
		{"RESULT_NOTICE",umath::to_integral(::sql::Result::Notice)},
		{"RESULT_WARNING",umath::to_integral(::sql::Result::Warning)},
		{"RESULT_ROW",umath::to_integral(::sql::Result::Row)},
		{"RESULT_DONE",umath::to_integral(::sql::Result::Done)},

		{"RESULT_IOERR_READ",umath::to_integral(::sql::Result::IOErrRead)},
		{"RESULT_IOERR_SHORT_READ",umath::to_integral(::sql::Result::IOErrShortRead)},
		{"RESULT_IOERR_WRITE",umath::to_integral(::sql::Result::IOErrWrite)},
		{"RESULT_IOERR_FSYNC",umath::to_integral(::sql::Result::IOErrFSync)},
		{"RESULT_IOERR_DIR_FSYNC",umath::to_integral(::sql::Result::IOErrDirFSync)},
		{"RESULT_IOERR_TRUNCATE",umath::to_integral(::sql::Result::IOErrTruncate)},
		{"RESULT_IOERR_FSTAT",umath::to_integral(::sql::Result::IOErrFStat)},
		{"RESULT_IOERR_UNLOCK",umath::to_integral(::sql::Result::IOErrUnlock)},
		{"RESULT_IOERR_RDLOCK",umath::to_integral(::sql::Result::IOErrRDLock)},
		{"RESULT_IOERR_DELETE",umath::to_integral(::sql::Result::IOErrDelete)},
		{"RESULT_IOERR_BLOCKED",umath::to_integral(::sql::Result::IOErrBlocked)},
		{"RESULT_IOERR_NOMEM",umath::to_integral(::sql::Result::IOErrNoMem)},
		{"RESULT_IOERR_ACCESS",umath::to_integral(::sql::Result::IOErrAccess)},
		{"RESULT_IOERR_CHECKRESERVEDLOCK",umath::to_integral(::sql::Result::IOErrCheckReservedLock)},
		{"RESULT_IOERR_LOCK",umath::to_integral(::sql::Result::IOErrLock)},
		{"RESULT_IOERR_CLOSE",umath::to_integral(::sql::Result::IOErrClose)},
		{"RESULT_IOERR_DIR_CLOSE",umath::to_integral(::sql::Result::IOErrDirClose)},
		{"RESULT_IOERR_SHMOPEN",umath::to_integral(::sql::Result::IOErrSHMOpen)},
		{"RESULT_IOERR_SHMSIZE",umath::to_integral(::sql::Result::IOErrSHMSize)},
		{"RESULT_IOERR_SHMLOCK",umath::to_integral(::sql::Result::IOErrSHMLock)},
		{"RESULT_IOERR_SHMMAP",umath::to_integral(::sql::Result::IOErrSHMMap)},
		{"RESULT_IOERR_SEEK",umath::to_integral(::sql::Result::IOErrSeek)},
		{"RESULT_IOERR_DELETE_NOENT",umath::to_integral(::sql::Result::IOErrDeleteNoENT)},
		{"RESULT_IOERR_MMAP",umath::to_integral(::sql::Result::IOErrMMap)},
		{"RESULT_IOERR_GETTEMPPATH",umath::to_integral(::sql::Result::IOErrGetTempPath)},
		{"RESULT_IOERR_CONVPATH",umath::to_integral(::sql::Result::IOErrConvPath)},
		{"RESULT_IOERR_VNODE",umath::to_integral(::sql::Result::IOErrVNode)},
		{"RESULT_IOERR_AUTH",umath::to_integral(::sql::Result::IOErrAuth)},
		{"RESULT_LOCKED_SHAREDCACHE",umath::to_integral(::sql::Result::LockedSharedCache)},
		{"RESULT_BUSY_RECOVERY",umath::to_integral(::sql::Result::BusyRecovery)},

		{"RESULT_BUSY_SNAPSHOT",umath::to_integral(::sql::Result::BusySnapshot)},
		{"RESULT_CANTOPEN_NOTEMPDIR",umath::to_integral(::sql::Result::CantOpenNoTempDir)},
		{"RESULT_CANTOPEN_ISDIR",umath::to_integral(::sql::Result::CantOpenIsDir)},
		{"RESULT_CANTOPEN_FULLPATH",umath::to_integral(::sql::Result::CantOpenFullPath)},
		{"RESULT_CANTOPEN_CONVPATH",umath::to_integral(::sql::Result::CantOpenConvPath)},
		{"RESULT_CORRUPT_VTAB",umath::to_integral(::sql::Result::CorruptVTab)},
		{"RESULT_READONLY_RECOVERY",umath::to_integral(::sql::Result::ReadOnlyRecovery)},
		{"RESULT_READONLY_CANTLOCK",umath::to_integral(::sql::Result::ReadOnlyCantLock)},
		{"RESULT_READONLY_ROLLBACK",umath::to_integral(::sql::Result::ReadOnlyRollback)},
		{"RESULT_READONLY_DBMOVED",umath::to_integral(::sql::Result::ReadOnlyDBMoved)},
		{"RESULT_ABORT_ROLLBACK",umath::to_integral(::sql::Result::AbortRollback)},
		{"RESULT_CONSTRAINT_CHECK",umath::to_integral(::sql::Result::ConstraintCheck)},
		{"RESULT_CONSTRAINT_COMMITHOOK",umath::to_integral(::sql::Result::ConstraintCommitHook)},
		{"RESULT_CONSTRAINT_FOREIGNKEY",umath::to_integral(::sql::Result::ConstraintForeignKey)},
		{"RESULT_CONSTRAINT_FUNCTION",umath::to_integral(::sql::Result::ConstraintFunction)},
		{"RESULT_CONSTRAINT_NOTNULL",umath::to_integral(::sql::Result::ConstraintNotNull)},
		{"RESULT_CONSTRAINT_PRIMARYKEY",umath::to_integral(::sql::Result::ConstraintPrimaryKey)},
		{"RESULT_CONSTRAINT_TRIGGER",umath::to_integral(::sql::Result::ConstraintTrigger)},
		{"RESULT_CONSTRAINT_UNIQUE",umath::to_integral(::sql::Result::ConstraintUnique)},
		{"RESULT_CONSTRAINT_VTAB",umath::to_integral(::sql::Result::ConstraintVTab)},
		{"RESULT_CONSTRAINT_ROWID",umath::to_integral(::sql::Result::ConstraintRowId)},
		{"RESULT_NOTICE_RECOVER_WAL",umath::to_integral(::sql::Result::NoticeRecoverWAL)},
		{"RESULT_NOTICE_RECOVER_ROLLBACK",umath::to_integral(::sql::Result::NoticeRecoverRollback)},
		{"RESULT_WARNING_AUTOINDEX",umath::to_integral(::sql::Result::WarningAutoIndex)},
		{"RESULT_AUTH_USER",umath::to_integral(::sql::Result::AuthUser)},
		{"RESULT_OK_LOAD_PERMANENTLY",umath::to_integral(::sql::Result::OkLoadPermanently)},

		{"RESULT_ER_ERROR_FIRST",umath::to_integral(::sql::Result::ErErrorFirst)},
		{"RESULT_ER_HASHCHK",umath::to_integral(::sql::Result::ErHashchk)},
		{"RESULT_ER_NISAMCHK",umath::to_integral(::sql::Result::ErNisamchk)},
		{"RESULT_ER_NO",umath::to_integral(::sql::Result::ErNo)},
		{"RESULT_ER_YES",umath::to_integral(::sql::Result::ErYes)},
		{"RESULT_ER_CANT_CREATE_FILE",umath::to_integral(::sql::Result::ErCantCreateFile)},
		{"RESULT_ER_CANT_CREATE_TABLE",umath::to_integral(::sql::Result::ErCantCreateTable)},
		{"RESULT_ER_CANT_CREATE_DB",umath::to_integral(::sql::Result::ErCantCreateDb)},
		{"RESULT_ER_DB_CREATE_EXISTS",umath::to_integral(::sql::Result::ErDbCreateExists)},
		{"RESULT_ER_DB_DROP_EXISTS",umath::to_integral(::sql::Result::ErDbDropExists)},
		{"RESULT_ER_DB_DROP_DELETE",umath::to_integral(::sql::Result::ErDbDropDelete)},
		{"RESULT_ER_DB_DROP_RMDIR",umath::to_integral(::sql::Result::ErDbDropRmdir)},
		{"RESULT_ER_CANT_DELETE_FILE",umath::to_integral(::sql::Result::ErCantDeleteFile)},
		{"RESULT_ER_CANT_FIND_SYSTEM_REC",umath::to_integral(::sql::Result::ErCantFindSystemRec)},
		{"RESULT_ER_CANT_GET_STAT",umath::to_integral(::sql::Result::ErCantGetStat)},
		{"RESULT_ER_CANT_GET_WD",umath::to_integral(::sql::Result::ErCantGetWd)},
		{"RESULT_ER_CANT_LOCK",umath::to_integral(::sql::Result::ErCantLock)},
		{"RESULT_ER_CANT_OPEN_FILE",umath::to_integral(::sql::Result::ErCantOpenFile)},
		{"RESULT_ER_FILE_NOT_FOUND",umath::to_integral(::sql::Result::ErFileNotFound)},
		{"RESULT_ER_CANT_READ_DIR",umath::to_integral(::sql::Result::ErCantReadDir)},
		{"RESULT_ER_CANT_SET_WD",umath::to_integral(::sql::Result::ErCantSetWd)},
		{"RESULT_ER_CHECKREAD",umath::to_integral(::sql::Result::ErCheckread)},
		{"RESULT_ER_DISK_FULL",umath::to_integral(::sql::Result::ErDiskFull)},
		{"RESULT_ER_DUP_KEY",umath::to_integral(::sql::Result::ErDupKey)},
		{"RESULT_ER_ERROR_ON_CLOSE",umath::to_integral(::sql::Result::ErErrorOnClose)},
		{"RESULT_ER_ERROR_ON_READ",umath::to_integral(::sql::Result::ErErrorOnRead)},
		{"RESULT_ER_ERROR_ON_RENAME",umath::to_integral(::sql::Result::ErErrorOnRename)},
		{"RESULT_ER_ERROR_ON_WRITE",umath::to_integral(::sql::Result::ErErrorOnWrite)},
		{"RESULT_ER_FILE_USED",umath::to_integral(::sql::Result::ErFileUsed)},
		{"RESULT_ER_FILSORT_ABORT",umath::to_integral(::sql::Result::ErFilsortAbort)},
		{"RESULT_ER_FORM_NOT_FOUND",umath::to_integral(::sql::Result::ErFormNotFound)},
		{"RESULT_ER_GET_ERRNO",umath::to_integral(::sql::Result::ErGetErrno)},
		{"RESULT_ER_ILLEGAL_HA",umath::to_integral(::sql::Result::ErIllegalHa)},
		{"RESULT_ER_KEY_NOT_FOUND",umath::to_integral(::sql::Result::ErKeyNotFound)},
		{"RESULT_ER_NOT_FORM_FILE",umath::to_integral(::sql::Result::ErNotFormFile)},
		{"RESULT_ER_NOT_KEYFILE",umath::to_integral(::sql::Result::ErNotKeyfile)},
		{"RESULT_ER_OLD_KEYFILE",umath::to_integral(::sql::Result::ErOldKeyfile)},
		{"RESULT_ER_OPEN_AS_READONLY",umath::to_integral(::sql::Result::ErOpenAsReadonly)},
		{"RESULT_ER_OUTOFMEMORY",umath::to_integral(::sql::Result::ErOutofmemory)},
		{"RESULT_ER_OUT_OF_SORTMEMORY",umath::to_integral(::sql::Result::ErOutOfSortmemory)},
		{"RESULT_ER_UNEXPECTED_EOF",umath::to_integral(::sql::Result::ErUnexpectedEof)},
		{"RESULT_ER_CON_COUNT_ERROR",umath::to_integral(::sql::Result::ErConCountError)},
		{"RESULT_ER_OUT_OF_RESOURCES",umath::to_integral(::sql::Result::ErOutOfResources)},
		{"RESULT_ER_BAD_HOST_ERROR",umath::to_integral(::sql::Result::ErBadHostError)},
		{"RESULT_ER_HANDSHAKE_ERROR",umath::to_integral(::sql::Result::ErHandshakeError)},
		{"RESULT_ER_DBACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErDbaccessDeniedError)},
		{"RESULT_ER_ACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErAccessDeniedError)},
		{"RESULT_ER_NO_DB_ERROR",umath::to_integral(::sql::Result::ErNoDbError)},
		{"RESULT_ER_UNKNOWN_COM_ERROR",umath::to_integral(::sql::Result::ErUnknownComError)},
		{"RESULT_ER_BAD_NULL_ERROR",umath::to_integral(::sql::Result::ErBadNullError)},
		{"RESULT_ER_BAD_DB_ERROR",umath::to_integral(::sql::Result::ErBadDbError)},
		{"RESULT_ER_TABLE_EXISTS_ERROR",umath::to_integral(::sql::Result::ErTableExistsError)},
		{"RESULT_ER_BAD_TABLE_ERROR",umath::to_integral(::sql::Result::ErBadTableError)},
		{"RESULT_ER_NON_UNIQ_ERROR",umath::to_integral(::sql::Result::ErNonUniqError)},
		{"RESULT_ER_SERVER_SHUTDOWN",umath::to_integral(::sql::Result::ErServerShutdown)},
		{"RESULT_ER_BAD_FIELD_ERROR",umath::to_integral(::sql::Result::ErBadFieldError)},
		{"RESULT_ER_WRONG_FIELD_WITH_GROUP",umath::to_integral(::sql::Result::ErWrongFieldWithGroup)},
		{"RESULT_ER_WRONG_GROUP_FIELD",umath::to_integral(::sql::Result::ErWrongGroupField)},
		{"RESULT_ER_WRONG_SUM_SELECT",umath::to_integral(::sql::Result::ErWrongSumSelect)},
		{"RESULT_ER_WRONG_VALUE_COUNT",umath::to_integral(::sql::Result::ErWrongValueCount)},
		{"RESULT_ER_TOO_LONG_IDENT",umath::to_integral(::sql::Result::ErTooLongIdent)},
		{"RESULT_ER_DUP_FIELDNAME",umath::to_integral(::sql::Result::ErDupFieldname)},
		{"RESULT_ER_DUP_KEYNAME",umath::to_integral(::sql::Result::ErDupKeyname)},
		{"RESULT_ER_DUP_ENTRY",umath::to_integral(::sql::Result::ErDupEntry)},
		{"RESULT_ER_WRONG_FIELD_SPEC",umath::to_integral(::sql::Result::ErWrongFieldSpec)},
		{"RESULT_ER_PARSE_ERROR",umath::to_integral(::sql::Result::ErParseError)},
		{"RESULT_ER_EMPTY_QUERY",umath::to_integral(::sql::Result::ErEmptyQuery)},
		{"RESULT_ER_NONUNIQ_TABLE",umath::to_integral(::sql::Result::ErNonuniqTable)},
		{"RESULT_ER_INVALID_DEFAULT",umath::to_integral(::sql::Result::ErInvalidDefault)},
		{"RESULT_ER_MULTIPLE_PRI_KEY",umath::to_integral(::sql::Result::ErMultiplePriKey)},
		{"RESULT_ER_TOO_MANY_KEYS",umath::to_integral(::sql::Result::ErTooManyKeys)},
		{"RESULT_ER_TOO_MANY_KEY_PARTS",umath::to_integral(::sql::Result::ErTooManyKeyParts)},
		{"RESULT_ER_TOO_LONG_KEY",umath::to_integral(::sql::Result::ErTooLongKey)},
		{"RESULT_ER_KEY_COLUMN_DOES_NOT_EXITS",umath::to_integral(::sql::Result::ErKeyColumnDoesNotExits)},
		{"RESULT_ER_BLOB_USED_AS_KEY",umath::to_integral(::sql::Result::ErBlobUsedAsKey)},
		{"RESULT_ER_TOO_BIG_FIELDLENGTH",umath::to_integral(::sql::Result::ErTooBigFieldlength)},
		{"RESULT_ER_WRONG_AUTO_KEY",umath::to_integral(::sql::Result::ErWrongAutoKey)},
		{"RESULT_ER_READY",umath::to_integral(::sql::Result::ErReady)},
		{"RESULT_ER_NORMAL_SHUTDOWN",umath::to_integral(::sql::Result::ErNormalShutdown)},
		{"RESULT_ER_GOT_SIGNAL",umath::to_integral(::sql::Result::ErGotSignal)},
		{"RESULT_ER_SHUTDOWN_COMPLETE",umath::to_integral(::sql::Result::ErShutdownComplete)},
		{"RESULT_ER_FORCING_CLOSE",umath::to_integral(::sql::Result::ErForcingClose)},
		{"RESULT_ER_IPSOCK_ERROR",umath::to_integral(::sql::Result::ErIpsockError)},
		{"RESULT_ER_NO_SUCH_INDEX",umath::to_integral(::sql::Result::ErNoSuchIndex)},
		{"RESULT_ER_WRONG_FIELD_TERMINATORS",umath::to_integral(::sql::Result::ErWrongFieldTerminators)},
		{"RESULT_ER_BLOBS_AND_NO_TERMINATED",umath::to_integral(::sql::Result::ErBlobsAndNoTerminated)},
		{"RESULT_ER_TEXTFILE_NOT_READABLE",umath::to_integral(::sql::Result::ErTextfileNotReadable)},
		{"RESULT_ER_FILE_EXISTS_ERROR",umath::to_integral(::sql::Result::ErFileExistsError)},
		{"RESULT_ER_LOAD_INFO",umath::to_integral(::sql::Result::ErLoadInfo)},
		{"RESULT_ER_ALTER_INFO",umath::to_integral(::sql::Result::ErAlterInfo)},
		{"RESULT_ER_WRONG_SUB_KEY",umath::to_integral(::sql::Result::ErWrongSubKey)},
		{"RESULT_ER_CANT_REMOVE_ALL_FIELDS",umath::to_integral(::sql::Result::ErCantRemoveAllFields)},
		{"RESULT_ER_CANT_DROP_FIELD_OR_KEY",umath::to_integral(::sql::Result::ErCantDropFieldOrKey)},
		{"RESULT_ER_INSERT_INFO",umath::to_integral(::sql::Result::ErInsertInfo)},
		{"RESULT_ER_UPDATE_TABLE_USED",umath::to_integral(::sql::Result::ErUpdateTableUsed)},
		{"RESULT_ER_NO_SUCH_THREAD",umath::to_integral(::sql::Result::ErNoSuchThread)},
		{"RESULT_ER_KILL_DENIED_ERROR",umath::to_integral(::sql::Result::ErKillDeniedError)},
		{"RESULT_ER_NO_TABLES_USED",umath::to_integral(::sql::Result::ErNoTablesUsed)},
		{"RESULT_ER_TOO_BIG_SET",umath::to_integral(::sql::Result::ErTooBigSet)},
		{"RESULT_ER_NO_UNIQUE_LOGFILE",umath::to_integral(::sql::Result::ErNoUniqueLogfile)},
		{"RESULT_ER_TABLE_NOT_LOCKED_FOR_WRITE",umath::to_integral(::sql::Result::ErTableNotLockedForWrite)},
		{"RESULT_ER_TABLE_NOT_LOCKED",umath::to_integral(::sql::Result::ErTableNotLocked)},
		{"RESULT_ER_BLOB_CANT_HAVE_DEFAULT",umath::to_integral(::sql::Result::ErBlobCantHaveDefault)},
		{"RESULT_ER_WRONG_DB_NAME",umath::to_integral(::sql::Result::ErWrongDbName)},
		{"RESULT_ER_WRONG_TABLE_NAME",umath::to_integral(::sql::Result::ErWrongTableName)},
		{"RESULT_ER_TOO_BIG_SELECT",umath::to_integral(::sql::Result::ErTooBigSelect)},
		{"RESULT_ER_UNKNOWN_ERROR",umath::to_integral(::sql::Result::ErUnknownError)},
		{"RESULT_ER_UNKNOWN_PROCEDURE",umath::to_integral(::sql::Result::ErUnknownProcedure)},
		{"RESULT_ER_WRONG_PARAMCOUNT_TO_PROCEDURE",umath::to_integral(::sql::Result::ErWrongParamcountToProcedure)},
		{"RESULT_ER_WRONG_PARAMETERS_TO_PROCEDURE",umath::to_integral(::sql::Result::ErWrongParametersToProcedure)},
		{"RESULT_ER_UNKNOWN_TABLE",umath::to_integral(::sql::Result::ErUnknownTable)},
		{"RESULT_ER_FIELD_SPECIFIED_TWICE",umath::to_integral(::sql::Result::ErFieldSpecifiedTwice)},
		{"RESULT_ER_INVALID_GROUP_FUNC_USE",umath::to_integral(::sql::Result::ErInvalidGroupFuncUse)},
		{"RESULT_ER_UNSUPPORTED_EXTENSION",umath::to_integral(::sql::Result::ErUnsupportedExtension)},
		{"RESULT_ER_TABLE_MUST_HAVE_COLUMNS",umath::to_integral(::sql::Result::ErTableMustHaveColumns)},
		{"RESULT_ER_RECORD_FILE_FULL",umath::to_integral(::sql::Result::ErRecordFileFull)},
		{"RESULT_ER_UNKNOWN_CHARACTER_SET",umath::to_integral(::sql::Result::ErUnknownCharacterSet)},
		{"RESULT_ER_TOO_MANY_TABLES",umath::to_integral(::sql::Result::ErTooManyTables)},
		{"RESULT_ER_TOO_MANY_FIELDS",umath::to_integral(::sql::Result::ErTooManyFields)},
		{"RESULT_ER_TOO_BIG_ROWSIZE",umath::to_integral(::sql::Result::ErTooBigRowsize)},
		{"RESULT_ER_STACK_OVERRUN",umath::to_integral(::sql::Result::ErStackOverrun)},
		{"RESULT_ER_WRONG_OUTER_JOIN",umath::to_integral(::sql::Result::ErWrongOuterJoin)},
		{"RESULT_ER_NULL_COLUMN_IN_INDEX",umath::to_integral(::sql::Result::ErNullColumnInIndex)},
		{"RESULT_ER_CANT_FIND_UDF",umath::to_integral(::sql::Result::ErCantFindUdf)},
		{"RESULT_ER_CANT_INITIALIZE_UDF",umath::to_integral(::sql::Result::ErCantInitializeUdf)},
		{"RESULT_ER_UDF_NO_PATHS",umath::to_integral(::sql::Result::ErUdfNoPaths)},
		{"RESULT_ER_UDF_EXISTS",umath::to_integral(::sql::Result::ErUdfExists)},
		{"RESULT_ER_CANT_OPEN_LIBRARY",umath::to_integral(::sql::Result::ErCantOpenLibrary)},
		{"RESULT_ER_CANT_FIND_DL_ENTRY",umath::to_integral(::sql::Result::ErCantFindDlEntry)},
		{"RESULT_ER_FUNCTION_NOT_DEFINED",umath::to_integral(::sql::Result::ErFunctionNotDefined)},
		{"RESULT_ER_HOST_IS_BLOCKED",umath::to_integral(::sql::Result::ErHostIsBlocked)},
		{"RESULT_ER_HOST_NOT_PRIVILEGED",umath::to_integral(::sql::Result::ErHostNotPrivileged)},
		{"RESULT_ER_PASSWORD_ANONYMOUS_USER",umath::to_integral(::sql::Result::ErPasswordAnonymousUser)},
		{"RESULT_ER_PASSWORD_NOT_ALLOWED",umath::to_integral(::sql::Result::ErPasswordNotAllowed)},
		{"RESULT_ER_PASSWORD_NO_MATCH",umath::to_integral(::sql::Result::ErPasswordNoMatch)},
		{"RESULT_ER_UPDATE_INFO",umath::to_integral(::sql::Result::ErUpdateInfo)},
		{"RESULT_ER_CANT_CREATE_THREAD",umath::to_integral(::sql::Result::ErCantCreateThread)},
		{"RESULT_ER_WRONG_VALUE_COUNT_ON_ROW",umath::to_integral(::sql::Result::ErWrongValueCountOnRow)},
		{"RESULT_ER_CANT_REOPEN_TABLE",umath::to_integral(::sql::Result::ErCantReopenTable)},
		{"RESULT_ER_INVALID_USE_OF_NULL",umath::to_integral(::sql::Result::ErInvalidUseOfNull)},
		{"RESULT_ER_REGEXP_ERROR",umath::to_integral(::sql::Result::ErRegexpError)},
		{"RESULT_ER_MIX_OF_GROUP_FUNC_AND_FIELDS",umath::to_integral(::sql::Result::ErMixOfGroupFuncAndFields)},
		{"RESULT_ER_NONEXISTING_GRANT",umath::to_integral(::sql::Result::ErNonexistingGrant)},
		{"RESULT_ER_TABLEACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErTableaccessDeniedError)},
		{"RESULT_ER_COLUMNACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErColumnaccessDeniedError)},
		{"RESULT_ER_ILLEGAL_GRANT_FOR_TABLE",umath::to_integral(::sql::Result::ErIllegalGrantForTable)},
		{"RESULT_ER_GRANT_WRONG_HOST_OR_USER",umath::to_integral(::sql::Result::ErGrantWrongHostOrUser)},
		{"RESULT_ER_NO_SUCH_TABLE",umath::to_integral(::sql::Result::ErNoSuchTable)},
		{"RESULT_ER_NONEXISTING_TABLE_GRANT",umath::to_integral(::sql::Result::ErNonexistingTableGrant)},
		{"RESULT_ER_NOT_ALLOWED_COMMAND",umath::to_integral(::sql::Result::ErNotAllowedCommand)},
		{"RESULT_ER_SYNTAX_ERROR",umath::to_integral(::sql::Result::ErSyntaxError)},
		{"RESULT_ER_DELAYED_CANT_CHANGE_LOCK",umath::to_integral(::sql::Result::ErDelayedCantChangeLock)},
		{"RESULT_ER_TOO_MANY_DELAYED_THREADS",umath::to_integral(::sql::Result::ErTooManyDelayedThreads)},
		{"RESULT_ER_ABORTING_CONNECTION",umath::to_integral(::sql::Result::ErAbortingConnection)},
		{"RESULT_ER_NET_PACKET_TOO_LARGE",umath::to_integral(::sql::Result::ErNetPacketTooLarge)},
		{"RESULT_ER_NET_READ_ERROR_FROM_PIPE",umath::to_integral(::sql::Result::ErNetReadErrorFromPipe)},
		{"RESULT_ER_NET_FCNTL_ERROR",umath::to_integral(::sql::Result::ErNetFcntlError)},
		{"RESULT_ER_NET_PACKETS_OUT_OF_ORDER",umath::to_integral(::sql::Result::ErNetPacketsOutOfOrder)},
		{"RESULT_ER_NET_UNCOMPRESS_ERROR",umath::to_integral(::sql::Result::ErNetUncompressError)},
		{"RESULT_ER_NET_READ_ERROR",umath::to_integral(::sql::Result::ErNetReadError)},
		{"RESULT_ER_NET_READ_INTERRUPTED",umath::to_integral(::sql::Result::ErNetReadInterrupted)},
		{"RESULT_ER_NET_ERROR_ON_WRITE",umath::to_integral(::sql::Result::ErNetErrorOnWrite)},
		{"RESULT_ER_NET_WRITE_INTERRUPTED",umath::to_integral(::sql::Result::ErNetWriteInterrupted)},
		{"RESULT_ER_TOO_LONG_STRING",umath::to_integral(::sql::Result::ErTooLongString)},
		{"RESULT_ER_TABLE_CANT_HANDLE_BLOB",umath::to_integral(::sql::Result::ErTableCantHandleBlob)},
		{"RESULT_ER_TABLE_CANT_HANDLE_AUTO_INCREMENT",umath::to_integral(::sql::Result::ErTableCantHandleAutoIncrement)},
		{"RESULT_ER_DELAYED_INSERT_TABLE_LOCKED",umath::to_integral(::sql::Result::ErDelayedInsertTableLocked)},
		{"RESULT_ER_WRONG_COLUMN_NAME",umath::to_integral(::sql::Result::ErWrongColumnName)},
		{"RESULT_ER_WRONG_KEY_COLUMN",umath::to_integral(::sql::Result::ErWrongKeyColumn)},
		{"RESULT_ER_WRONG_MRG_TABLE",umath::to_integral(::sql::Result::ErWrongMrgTable)},
		{"RESULT_ER_DUP_UNIQUE",umath::to_integral(::sql::Result::ErDupUnique)},
		{"RESULT_ER_BLOB_KEY_WITHOUT_LENGTH",umath::to_integral(::sql::Result::ErBlobKeyWithoutLength)},
		{"RESULT_ER_PRIMARY_CANT_HAVE_NULL",umath::to_integral(::sql::Result::ErPrimaryCantHaveNull)},
		{"RESULT_ER_TOO_MANY_ROWS",umath::to_integral(::sql::Result::ErTooManyRows)},
		{"RESULT_ER_REQUIRES_PRIMARY_KEY",umath::to_integral(::sql::Result::ErRequiresPrimaryKey)},
		{"RESULT_ER_NO_RAID_COMPILED",umath::to_integral(::sql::Result::ErNoRaidCompiled)},
		{"RESULT_ER_UPDATE_WITHOUT_KEY_IN_SAFE_MODE",umath::to_integral(::sql::Result::ErUpdateWithoutKeyInSafeMode)},
		{"RESULT_ER_KEY_DOES_NOT_EXITS",umath::to_integral(::sql::Result::ErKeyDoesNotExits)},
		{"RESULT_ER_CHECK_NO_SUCH_TABLE",umath::to_integral(::sql::Result::ErCheckNoSuchTable)},
		{"RESULT_ER_CHECK_NOT_IMPLEMENTED",umath::to_integral(::sql::Result::ErCheckNotImplemented)},
		{"RESULT_ER_CANT_DO_THIS_DURING_AN_TRANSACTION",umath::to_integral(::sql::Result::ErCantDoThisDuringAnTransaction)},
		{"RESULT_ER_ERROR_DURING_COMMIT",umath::to_integral(::sql::Result::ErErrorDuringCommit)},
		{"RESULT_ER_ERROR_DURING_ROLLBACK",umath::to_integral(::sql::Result::ErErrorDuringRollback)},
		{"RESULT_ER_ERROR_DURING_FLUSH_LOGS",umath::to_integral(::sql::Result::ErErrorDuringFlushLogs)},
		{"RESULT_ER_ERROR_DURING_CHECKPOINT",umath::to_integral(::sql::Result::ErErrorDuringCheckpoint)},
		{"RESULT_ER_NEW_ABORTING_CONNECTION",umath::to_integral(::sql::Result::ErNewAbortingConnection)},
		{"RESULT_ER_DUMP_NOT_IMPLEMENTED",umath::to_integral(::sql::Result::ErDumpNotImplemented)},
		{"RESULT_ER_FLUSH_MASTER_BINLOG_CLOSED",umath::to_integral(::sql::Result::ErFlushMasterBinlogClosed)},
		{"RESULT_ER_INDEX_REBUILD",umath::to_integral(::sql::Result::ErIndexRebuild)},
		{"RESULT_ER_MASTER",umath::to_integral(::sql::Result::ErMaster)},
		{"RESULT_ER_MASTER_NET_READ",umath::to_integral(::sql::Result::ErMasterNetRead)},
		{"RESULT_ER_MASTER_NET_WRITE",umath::to_integral(::sql::Result::ErMasterNetWrite)},
		{"RESULT_ER_FT_MATCHING_KEY_NOT_FOUND",umath::to_integral(::sql::Result::ErFtMatchingKeyNotFound)},
		{"RESULT_ER_LOCK_OR_ACTIVE_TRANSACTION",umath::to_integral(::sql::Result::ErLockOrActiveTransaction)},
		{"RESULT_ER_UNKNOWN_SYSTEM_VARIABLE",umath::to_integral(::sql::Result::ErUnknownSystemVariable)},
		{"RESULT_ER_CRASHED_ON_USAGE",umath::to_integral(::sql::Result::ErCrashedOnUsage)},
		{"RESULT_ER_CRASHED_ON_REPAIR",umath::to_integral(::sql::Result::ErCrashedOnRepair)},
		{"RESULT_ER_WARNING_NOT_COMPLETE_ROLLBACK",umath::to_integral(::sql::Result::ErWarningNotCompleteRollback)},
		{"RESULT_ER_TRANS_CACHE_FULL",umath::to_integral(::sql::Result::ErTransCacheFull)},
		{"RESULT_ER_SLAVE_MUST_STOP",umath::to_integral(::sql::Result::ErSlaveMustStop)},
		{"RESULT_ER_SLAVE_NOT_RUNNING",umath::to_integral(::sql::Result::ErSlaveNotRunning)},
		{"RESULT_ER_BAD_SLAVE",umath::to_integral(::sql::Result::ErBadSlave)},
		{"RESULT_ER_MASTER_INFO",umath::to_integral(::sql::Result::ErMasterInfo)},
		{"RESULT_ER_SLAVE_THREAD",umath::to_integral(::sql::Result::ErSlaveThread)},
		{"RESULT_ER_TOO_MANY_USER_CONNECTIONS",umath::to_integral(::sql::Result::ErTooManyUserConnections)},
		{"RESULT_ER_SET_CONSTANTS_ONLY",umath::to_integral(::sql::Result::ErSetConstantsOnly)},
		{"RESULT_ER_LOCK_WAIT_TIMEOUT",umath::to_integral(::sql::Result::ErLockWaitTimeout)},
		{"RESULT_ER_LOCK_TABLE_FULL",umath::to_integral(::sql::Result::ErLockTableFull)},
		{"RESULT_ER_READ_ONLY_TRANSACTION",umath::to_integral(::sql::Result::ErReadOnlyTransaction)},
		{"RESULT_ER_DROP_DB_WITH_READ_LOCK",umath::to_integral(::sql::Result::ErDropDbWithReadLock)},
		{"RESULT_ER_CREATE_DB_WITH_READ_LOCK",umath::to_integral(::sql::Result::ErCreateDbWithReadLock)},
		{"RESULT_ER_WRONG_ARGUMENTS",umath::to_integral(::sql::Result::ErWrongArguments)},
		{"RESULT_ER_NO_PERMISSION_TO_CREATE_USER",umath::to_integral(::sql::Result::ErNoPermissionToCreateUser)},
		{"RESULT_ER_UNION_TABLES_IN_DIFFERENT_DIR",umath::to_integral(::sql::Result::ErUnionTablesInDifferentDir)},
		{"RESULT_ER_LOCK_DEADLOCK",umath::to_integral(::sql::Result::ErLockDeadlock)},
		{"RESULT_ER_TABLE_CANT_HANDLE_FT",umath::to_integral(::sql::Result::ErTableCantHandleFt)},
		{"RESULT_ER_CANNOT_ADD_FOREIGN",umath::to_integral(::sql::Result::ErCannotAddForeign)},
		{"RESULT_ER_NO_REFERENCED_ROW",umath::to_integral(::sql::Result::ErNoReferencedRow)},
		{"RESULT_ER_ROW_IS_REFERENCED",umath::to_integral(::sql::Result::ErRowIsReferenced)},
		{"RESULT_ER_CONNECT_TO_MASTER",umath::to_integral(::sql::Result::ErConnectToMaster)},
		{"RESULT_ER_QUERY_ON_MASTER",umath::to_integral(::sql::Result::ErQueryOnMaster)},
		{"RESULT_ER_ERROR_WHEN_EXECUTING_COMMAND",umath::to_integral(::sql::Result::ErErrorWhenExecutingCommand)},
		{"RESULT_ER_WRONG_USAGE",umath::to_integral(::sql::Result::ErWrongUsage)},
		{"RESULT_ER_WRONG_NUMBER_OF_COLUMNS_IN_SELECT",umath::to_integral(::sql::Result::ErWrongNumberOfColumnsInSelect)},
		{"RESULT_ER_CANT_UPDATE_WITH_READLOCK",umath::to_integral(::sql::Result::ErCantUpdateWithReadlock)},
		{"RESULT_ER_MIXING_NOT_ALLOWED",umath::to_integral(::sql::Result::ErMixingNotAllowed)},
		{"RESULT_ER_DUP_ARGUMENT",umath::to_integral(::sql::Result::ErDupArgument)},
		{"RESULT_ER_USER_LIMIT_REACHED",umath::to_integral(::sql::Result::ErUserLimitReached)},
		{"RESULT_ER_SPECIFIC_ACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErSpecificAccessDeniedError)},
		{"RESULT_ER_LOCAL_VARIABLE",umath::to_integral(::sql::Result::ErLocalVariable)},
		{"RESULT_ER_GLOBAL_VARIABLE",umath::to_integral(::sql::Result::ErGlobalVariable)},
		{"RESULT_ER_NO_DEFAULT",umath::to_integral(::sql::Result::ErNoDefault)},
		{"RESULT_ER_WRONG_VALUE_FOR_VAR",umath::to_integral(::sql::Result::ErWrongValueForVar)},
		{"RESULT_ER_WRONG_TYPE_FOR_VAR",umath::to_integral(::sql::Result::ErWrongTypeForVar)},
		{"RESULT_ER_VAR_CANT_BE_READ",umath::to_integral(::sql::Result::ErVarCantBeRead)},
		{"RESULT_ER_CANT_USE_OPTION_HERE",umath::to_integral(::sql::Result::ErCantUseOptionHere)},
		{"RESULT_ER_NOT_SUPPORTED_YET",umath::to_integral(::sql::Result::ErNotSupportedYet)},
		{"RESULT_ER_MASTER_FATAL_ERROR_READING_BINLOG",umath::to_integral(::sql::Result::ErMasterFatalErrorReadingBinlog)},
		{"RESULT_ER_SLAVE_IGNORED_TABLE",umath::to_integral(::sql::Result::ErSlaveIgnoredTable)},
		{"RESULT_ER_INCORRECT_GLOBAL_LOCAL_VAR",umath::to_integral(::sql::Result::ErIncorrectGlobalLocalVar)},
		{"RESULT_ER_WRONG_FK_DEF",umath::to_integral(::sql::Result::ErWrongFkDef)},
		{"RESULT_ER_KEY_REF_DO_NOT_MATCH_TABLE_REF",umath::to_integral(::sql::Result::ErKeyRefDoNotMatchTableRef)},
		{"RESULT_ER_OPERAND_COLUMNS",umath::to_integral(::sql::Result::ErOperandColumns)},
		{"RESULT_ER_SUBQUERY_NO_1_ROW",umath::to_integral(::sql::Result::ErSubqueryNo1Row)},
		{"RESULT_ER_UNKNOWN_STMT_HANDLER",umath::to_integral(::sql::Result::ErUnknownStmtHandler)},
		{"RESULT_ER_CORRUPT_HELP_DB",umath::to_integral(::sql::Result::ErCorruptHelpDb)},
		{"RESULT_ER_CYCLIC_REFERENCE",umath::to_integral(::sql::Result::ErCyclicReference)},
		{"RESULT_ER_AUTO_CONVERT",umath::to_integral(::sql::Result::ErAutoConvert)},
		{"RESULT_ER_ILLEGAL_REFERENCE",umath::to_integral(::sql::Result::ErIllegalReference)},
		{"RESULT_ER_DERIVED_MUST_HAVE_ALIAS",umath::to_integral(::sql::Result::ErDerivedMustHaveAlias)},
		{"RESULT_ER_SELECT_REDUCED",umath::to_integral(::sql::Result::ErSelectReduced)},
		{"RESULT_ER_TABLENAME_NOT_ALLOWED_HERE",umath::to_integral(::sql::Result::ErTablenameNotAllowedHere)},
		{"RESULT_ER_NOT_SUPPORTED_AUTH_MODE",umath::to_integral(::sql::Result::ErNotSupportedAuthMode)},
		{"RESULT_ER_SPATIAL_CANT_HAVE_NULL",umath::to_integral(::sql::Result::ErSpatialCantHaveNull)},
		{"RESULT_ER_COLLATION_CHARSET_MISMATCH",umath::to_integral(::sql::Result::ErCollationCharsetMismatch)},
		{"RESULT_ER_SLAVE_WAS_RUNNING",umath::to_integral(::sql::Result::ErSlaveWasRunning)},
		{"RESULT_ER_SLAVE_WAS_NOT_RUNNING",umath::to_integral(::sql::Result::ErSlaveWasNotRunning)},
		{"RESULT_ER_TOO_BIG_FOR_UNCOMPRESS",umath::to_integral(::sql::Result::ErTooBigForUncompress)},
		{"RESULT_ER_ZLIB_Z_MEM_ERROR",umath::to_integral(::sql::Result::ErZlibZMemError)},
		{"RESULT_ER_ZLIB_Z_BUF_ERROR",umath::to_integral(::sql::Result::ErZlibZBufError)},
		{"RESULT_ER_ZLIB_Z_DATA_ERROR",umath::to_integral(::sql::Result::ErZlibZDataError)},
		{"RESULT_ER_CUT_VALUE_GROUP_CONCAT",umath::to_integral(::sql::Result::ErCutValueGroupConcat)},
		{"RESULT_ER_WARN_TOO_FEW_RECORDS",umath::to_integral(::sql::Result::ErWarnTooFewRecords)},
		{"RESULT_ER_WARN_TOO_MANY_RECORDS",umath::to_integral(::sql::Result::ErWarnTooManyRecords)},
		{"RESULT_ER_WARN_NULL_TO_NOTNULL",umath::to_integral(::sql::Result::ErWarnNullToNotnull)},
		{"RESULT_ER_WARN_DATA_OUT_OF_RANGE",umath::to_integral(::sql::Result::ErWarnDataOutOfRange)},
		{"RESULT_WARN_DATA_TRUNCATED",umath::to_integral(::sql::Result::WarnDataTruncated)},
		{"RESULT_ER_WARN_USING_OTHER_HANDLER",umath::to_integral(::sql::Result::ErWarnUsingOtherHandler)},
		{"RESULT_ER_CANT_AGGREGATE_2COLLATIONS",umath::to_integral(::sql::Result::ErCantAggregate2collations)},
		{"RESULT_ER_DROP_USER",umath::to_integral(::sql::Result::ErDropUser)},
		{"RESULT_ER_REVOKE_GRANTS",umath::to_integral(::sql::Result::ErRevokeGrants)},
		{"RESULT_ER_CANT_AGGREGATE_3COLLATIONS",umath::to_integral(::sql::Result::ErCantAggregate3collations)},
		{"RESULT_ER_CANT_AGGREGATE_NCOLLATIONS",umath::to_integral(::sql::Result::ErCantAggregateNcollations)},
		{"RESULT_ER_VARIABLE_IS_NOT_STRUCT",umath::to_integral(::sql::Result::ErVariableIsNotStruct)},
		{"RESULT_ER_UNKNOWN_COLLATION",umath::to_integral(::sql::Result::ErUnknownCollation)},
		{"RESULT_ER_SLAVE_IGNORED_SSL_PARAMS",umath::to_integral(::sql::Result::ErSlaveIgnoredSslParams)},
		{"RESULT_ER_SERVER_IS_IN_SECURE_AUTH_MODE",umath::to_integral(::sql::Result::ErServerIsInSecureAuthMode)},
		{"RESULT_ER_WARN_FIELD_RESOLVED",umath::to_integral(::sql::Result::ErWarnFieldResolved)},
		{"RESULT_ER_BAD_SLAVE_UNTIL_COND",umath::to_integral(::sql::Result::ErBadSlaveUntilCond)},
		{"RESULT_ER_MISSING_SKIP_SLAVE",umath::to_integral(::sql::Result::ErMissingSkipSlave)},
		{"RESULT_ER_UNTIL_COND_IGNORED",umath::to_integral(::sql::Result::ErUntilCondIgnored)},
		{"RESULT_ER_WRONG_NAME_FOR_INDEX",umath::to_integral(::sql::Result::ErWrongNameForIndex)},
		{"RESULT_ER_WRONG_NAME_FOR_CATALOG",umath::to_integral(::sql::Result::ErWrongNameForCatalog)},
		{"RESULT_ER_WARN_QC_RESIZE",umath::to_integral(::sql::Result::ErWarnQcResize)},
		{"RESULT_ER_BAD_FT_COLUMN",umath::to_integral(::sql::Result::ErBadFtColumn)},
		{"RESULT_ER_UNKNOWN_KEY_CACHE",umath::to_integral(::sql::Result::ErUnknownKeyCache)},
		{"RESULT_ER_WARN_HOSTNAME_WONT_WORK",umath::to_integral(::sql::Result::ErWarnHostnameWontWork)},
		{"RESULT_ER_UNKNOWN_STORAGE_ENGINE",umath::to_integral(::sql::Result::ErUnknownStorageEngine)},
		{"RESULT_ER_WARN_DEPRECATED_SYNTAX",umath::to_integral(::sql::Result::ErWarnDeprecatedSyntax)},
		{"RESULT_ER_NON_UPDATABLE_TABLE",umath::to_integral(::sql::Result::ErNonUpdatableTable)},
		{"RESULT_ER_FEATURE_DISABLED",umath::to_integral(::sql::Result::ErFeatureDisabled)},
		{"RESULT_ER_OPTION_PREVENTS_STATEMENT",umath::to_integral(::sql::Result::ErOptionPreventsStatement)},
		{"RESULT_ER_DUPLICATED_VALUE_IN_TYPE",umath::to_integral(::sql::Result::ErDuplicatedValueInType)},
		{"RESULT_ER_TRUNCATED_WRONG_VALUE",umath::to_integral(::sql::Result::ErTruncatedWrongValue)},
		{"RESULT_ER_TOO_MUCH_AUTO_TIMESTAMP_COLS",umath::to_integral(::sql::Result::ErTooMuchAutoTimestampCols)},
		{"RESULT_ER_INVALID_ON_UPDATE",umath::to_integral(::sql::Result::ErInvalidOnUpdate)},
		{"RESULT_ER_UNSUPPORTED_PS",umath::to_integral(::sql::Result::ErUnsupportedPs)},
		{"RESULT_ER_GET_ERRMSG",umath::to_integral(::sql::Result::ErGetErrmsg)},
		{"RESULT_ER_GET_TEMPORARY_ERRMSG",umath::to_integral(::sql::Result::ErGetTemporaryErrmsg)},
		{"RESULT_ER_UNKNOWN_TIME_ZONE",umath::to_integral(::sql::Result::ErUnknownTimeZone)},
		{"RESULT_ER_WARN_INVALID_TIMESTAMP",umath::to_integral(::sql::Result::ErWarnInvalidTimestamp)},
		{"RESULT_ER_INVALID_CHARACTER_STRING",umath::to_integral(::sql::Result::ErInvalidCharacterString)},
		{"RESULT_ER_WARN_ALLOWED_PACKET_OVERFLOWED",umath::to_integral(::sql::Result::ErWarnAllowedPacketOverflowed)},
		{"RESULT_ER_CONFLICTING_DECLARATIONS",umath::to_integral(::sql::Result::ErConflictingDeclarations)},
		{"RESULT_ER_SP_NO_RECURSIVE_CREATE",umath::to_integral(::sql::Result::ErSpNoRecursiveCreate)},
		{"RESULT_ER_SP_ALREADY_EXISTS",umath::to_integral(::sql::Result::ErSpAlreadyExists)},
		{"RESULT_ER_SP_DOES_NOT_EXIST",umath::to_integral(::sql::Result::ErSpDoesNotExist)},
		{"RESULT_ER_SP_DROP_FAILED",umath::to_integral(::sql::Result::ErSpDropFailed)},
		{"RESULT_ER_SP_STORE_FAILED",umath::to_integral(::sql::Result::ErSpStoreFailed)},
		{"RESULT_ER_SP_LILABEL_MISMATCH",umath::to_integral(::sql::Result::ErSpLilabelMismatch)},
		{"RESULT_ER_SP_LABEL_REDEFINE",umath::to_integral(::sql::Result::ErSpLabelRedefine)},
		{"RESULT_ER_SP_LABEL_MISMATCH",umath::to_integral(::sql::Result::ErSpLabelMismatch)},
		{"RESULT_ER_SP_UNINIT_VAR",umath::to_integral(::sql::Result::ErSpUninitVar)},
		{"RESULT_ER_SP_BADSELECT",umath::to_integral(::sql::Result::ErSpBadselect)},
		{"RESULT_ER_SP_BADRETURN",umath::to_integral(::sql::Result::ErSpBadreturn)},
		{"RESULT_ER_SP_BADSTATEMENT",umath::to_integral(::sql::Result::ErSpBadstatement)},
		{"RESULT_ER_UPDATE_LOG_DEPRECATED_IGNORED",umath::to_integral(::sql::Result::ErUpdateLogDeprecatedIgnored)},
		{"RESULT_ER_UPDATE_LOG_DEPRECATED_TRANSLATED",umath::to_integral(::sql::Result::ErUpdateLogDeprecatedTranslated)},
		{"RESULT_ER_QUERY_INTERRUPTED",umath::to_integral(::sql::Result::ErQueryInterrupted)},
		{"RESULT_ER_SP_WRONG_NO_OF_ARGS",umath::to_integral(::sql::Result::ErSpWrongNoOfArgs)},
		{"RESULT_ER_SP_COND_MISMATCH",umath::to_integral(::sql::Result::ErSpCondMismatch)},
		{"RESULT_ER_SP_NORETURN",umath::to_integral(::sql::Result::ErSpNoreturn)},
		{"RESULT_ER_SP_NORETURNEND",umath::to_integral(::sql::Result::ErSpNoreturnend)},
		{"RESULT_ER_SP_BAD_CURSOR_QUERY",umath::to_integral(::sql::Result::ErSpBadCursorQuery)},
		{"RESULT_ER_SP_BAD_CURSOR_SELECT",umath::to_integral(::sql::Result::ErSpBadCursorSelect)},
		{"RESULT_ER_SP_CURSOR_MISMATCH",umath::to_integral(::sql::Result::ErSpCursorMismatch)},
		{"RESULT_ER_SP_CURSOR_ALREADY_OPEN",umath::to_integral(::sql::Result::ErSpCursorAlreadyOpen)},
		{"RESULT_ER_SP_CURSOR_NOT_OPEN",umath::to_integral(::sql::Result::ErSpCursorNotOpen)},
		{"RESULT_ER_SP_UNDECLARED_VAR",umath::to_integral(::sql::Result::ErSpUndeclaredVar)},
		{"RESULT_ER_SP_WRONG_NO_OF_FETCH_ARGS",umath::to_integral(::sql::Result::ErSpWrongNoOfFetchArgs)},
		{"RESULT_ER_SP_FETCH_NO_DATA",umath::to_integral(::sql::Result::ErSpFetchNoData)},
		{"RESULT_ER_SP_DUP_PARAM",umath::to_integral(::sql::Result::ErSpDupParam)},
		{"RESULT_ER_SP_DUP_VAR",umath::to_integral(::sql::Result::ErSpDupVar)},
		{"RESULT_ER_SP_DUP_COND",umath::to_integral(::sql::Result::ErSpDupCond)},
		{"RESULT_ER_SP_DUP_CURS",umath::to_integral(::sql::Result::ErSpDupCurs)},
		{"RESULT_ER_SP_CANT_ALTER",umath::to_integral(::sql::Result::ErSpCantAlter)},
		{"RESULT_ER_SP_SUBSELECT_NYI",umath::to_integral(::sql::Result::ErSpSubselectNyi)},
		{"RESULT_ER_STMT_NOT_ALLOWED_IN_SF_OR_TRG",umath::to_integral(::sql::Result::ErStmtNotAllowedInSfOrTrg)},
		{"RESULT_ER_SP_VARCOND_AFTER_CURSHNDLR",umath::to_integral(::sql::Result::ErSpVarcondAfterCurshndlr)},
		{"RESULT_ER_SP_CURSOR_AFTER_HANDLER",umath::to_integral(::sql::Result::ErSpCursorAfterHandler)},
		{"RESULT_ER_SP_CASE_NOT_FOUND",umath::to_integral(::sql::Result::ErSpCaseNotFound)},
		{"RESULT_ER_FPARSER_TOO_BIG_FILE",umath::to_integral(::sql::Result::ErFparserTooBigFile)},
		{"RESULT_ER_FPARSER_BAD_HEADER",umath::to_integral(::sql::Result::ErFparserBadHeader)},
		{"RESULT_ER_FPARSER_EOF_IN_COMMENT",umath::to_integral(::sql::Result::ErFparserEofInComment)},
		{"RESULT_ER_FPARSER_ERROR_IN_PARAMETER",umath::to_integral(::sql::Result::ErFparserErrorInParameter)},
		{"RESULT_ER_FPARSER_EOF_IN_UNKNOWN_PARAMETER",umath::to_integral(::sql::Result::ErFparserEofInUnknownParameter)},
		{"RESULT_ER_VIEW_NO_EXPLAIN",umath::to_integral(::sql::Result::ErViewNoExplain)},
		{"RESULT_ER_FRM_UNKNOWN_TYPE",umath::to_integral(::sql::Result::ErFrmUnknownType)},
		{"RESULT_ER_WRONG_OBJECT",umath::to_integral(::sql::Result::ErWrongObject)},
		{"RESULT_ER_NONUPDATEABLE_COLUMN",umath::to_integral(::sql::Result::ErNonupdateableColumn)},
		{"RESULT_ER_VIEW_SELECT_DERIVED",umath::to_integral(::sql::Result::ErViewSelectDerived)},
		{"RESULT_ER_VIEW_SELECT_CLAUSE",umath::to_integral(::sql::Result::ErViewSelectClause)},
		{"RESULT_ER_VIEW_SELECT_VARIABLE",umath::to_integral(::sql::Result::ErViewSelectVariable)},
		{"RESULT_ER_VIEW_SELECT_TMPTABLE",umath::to_integral(::sql::Result::ErViewSelectTmptable)},
		{"RESULT_ER_VIEW_WRONG_LIST",umath::to_integral(::sql::Result::ErViewWrongList)},
		{"RESULT_ER_WARN_VIEW_MERGE",umath::to_integral(::sql::Result::ErWarnViewMerge)},
		{"RESULT_ER_WARN_VIEW_WITHOUT_KEY",umath::to_integral(::sql::Result::ErWarnViewWithoutKey)},
		{"RESULT_ER_VIEW_INVALID",umath::to_integral(::sql::Result::ErViewInvalid)},
		{"RESULT_ER_SP_NO_DROP_SP",umath::to_integral(::sql::Result::ErSpNoDropSp)},
		{"RESULT_ER_SP_GOTO_IN_HNDLR",umath::to_integral(::sql::Result::ErSpGotoInHndlr)},
		{"RESULT_ER_TRG_ALREADY_EXISTS",umath::to_integral(::sql::Result::ErTrgAlreadyExists)},
		{"RESULT_ER_TRG_DOES_NOT_EXIST",umath::to_integral(::sql::Result::ErTrgDoesNotExist)},
		{"RESULT_ER_TRG_ON_VIEW_OR_TEMP_TABLE",umath::to_integral(::sql::Result::ErTrgOnViewOrTempTable)},
		{"RESULT_ER_TRG_CANT_CHANGE_ROW",umath::to_integral(::sql::Result::ErTrgCantChangeRow)},
		{"RESULT_ER_TRG_NO_SUCH_ROW_IN_TRG",umath::to_integral(::sql::Result::ErTrgNoSuchRowInTrg)},
		{"RESULT_ER_NO_DEFAULT_FOR_FIELD",umath::to_integral(::sql::Result::ErNoDefaultForField)},
		{"RESULT_ER_DIVISION_BY_ZERO",umath::to_integral(::sql::Result::ErDivisionByZero)},
		{"RESULT_ER_TRUNCATED_WRONG_VALUE_FOR_FIELD",umath::to_integral(::sql::Result::ErTruncatedWrongValueForField)},
		{"RESULT_ER_ILLEGAL_VALUE_FOR_TYPE",umath::to_integral(::sql::Result::ErIllegalValueForType)},
		{"RESULT_ER_VIEW_NONUPD_CHECK",umath::to_integral(::sql::Result::ErViewNonupdCheck)},
		{"RESULT_ER_VIEW_CHECK_FAILED",umath::to_integral(::sql::Result::ErViewCheckFailed)},
		{"RESULT_ER_PROCACCESS_DENIED_ERROR",umath::to_integral(::sql::Result::ErProcaccessDeniedError)},
		{"RESULT_ER_RELAY_LOG_FAIL",umath::to_integral(::sql::Result::ErRelayLogFail)},
		{"RESULT_ER_PASSWD_LENGTH",umath::to_integral(::sql::Result::ErPasswdLength)},
		{"RESULT_ER_UNKNOWN_TARGET_BINLOG",umath::to_integral(::sql::Result::ErUnknownTargetBinlog)},
		{"RESULT_ER_IO_ERR_LOG_INDEX_READ",umath::to_integral(::sql::Result::ErIoErrLogIndexRead)},
		{"RESULT_ER_BINLOG_PURGE_PROHIBITED",umath::to_integral(::sql::Result::ErBinlogPurgeProhibited)},
		{"RESULT_ER_FSEEK_FAIL",umath::to_integral(::sql::Result::ErFseekFail)},
		{"RESULT_ER_BINLOG_PURGE_FATAL_ERR",umath::to_integral(::sql::Result::ErBinlogPurgeFatalErr)},
		{"RESULT_ER_LOG_IN_USE",umath::to_integral(::sql::Result::ErLogInUse)},
		{"RESULT_ER_LOG_PURGE_UNKNOWN_ERR",umath::to_integral(::sql::Result::ErLogPurgeUnknownErr)},
		{"RESULT_ER_RELAY_LOG_INIT",umath::to_integral(::sql::Result::ErRelayLogInit)},
		{"RESULT_ER_NO_BINARY_LOGGING",umath::to_integral(::sql::Result::ErNoBinaryLogging)},
		{"RESULT_ER_RESERVED_SYNTAX",umath::to_integral(::sql::Result::ErReservedSyntax)},
		{"RESULT_ER_WSAS_FAILED",umath::to_integral(::sql::Result::ErWsasFailed)},
		{"RESULT_ER_DIFF_GROUPS_PROC",umath::to_integral(::sql::Result::ErDiffGroupsProc)},
		{"RESULT_ER_NO_GROUP_FOR_PROC",umath::to_integral(::sql::Result::ErNoGroupForProc)},
		{"RESULT_ER_ORDER_WITH_PROC",umath::to_integral(::sql::Result::ErOrderWithProc)},
		{"RESULT_ER_LOGGING_PROHIBIT_CHANGING_OF",umath::to_integral(::sql::Result::ErLoggingProhibitChangingOf)},
		{"RESULT_ER_NO_FILE_MAPPING",umath::to_integral(::sql::Result::ErNoFileMapping)},
		{"RESULT_ER_WRONG_MAGIC",umath::to_integral(::sql::Result::ErWrongMagic)},
		{"RESULT_ER_PS_MANY_PARAM",umath::to_integral(::sql::Result::ErPsManyParam)},
		{"RESULT_ER_KEY_PART_0",umath::to_integral(::sql::Result::ErKeyPart0)},
		{"RESULT_ER_VIEW_CHECKSUM",umath::to_integral(::sql::Result::ErViewChecksum)},
		{"RESULT_ER_VIEW_MULTIUPDATE",umath::to_integral(::sql::Result::ErViewMultiupdate)},
		{"RESULT_ER_VIEW_NO_INSERT_FIELD_LIST",umath::to_integral(::sql::Result::ErViewNoInsertFieldList)},
		{"RESULT_ER_VIEW_DELETE_MERGE_VIEW",umath::to_integral(::sql::Result::ErViewDeleteMergeView)},
		{"RESULT_ER_CANNOT_USER",umath::to_integral(::sql::Result::ErCannotUser)},
		{"RESULT_ER_XAER_NOTA",umath::to_integral(::sql::Result::ErXaerNota)},
		{"RESULT_ER_XAER_INVAL",umath::to_integral(::sql::Result::ErXaerInval)},
		{"RESULT_ER_XAER_RMFAIL",umath::to_integral(::sql::Result::ErXaerRmfail)},
		{"RESULT_ER_XAER_OUTSIDE",umath::to_integral(::sql::Result::ErXaerOutside)},
		{"RESULT_ER_XAER_RMERR",umath::to_integral(::sql::Result::ErXaerRmerr)},
		{"RESULT_ER_XA_RBROLLBACK",umath::to_integral(::sql::Result::ErXaRbrollback)},
		{"RESULT_ER_NONEXISTING_PROC_GRANT",umath::to_integral(::sql::Result::ErNonexistingProcGrant)},
		{"RESULT_ER_PROC_AUTO_GRANT_FAIL",umath::to_integral(::sql::Result::ErProcAutoGrantFail)},
		{"RESULT_ER_PROC_AUTO_REVOKE_FAIL",umath::to_integral(::sql::Result::ErProcAutoRevokeFail)},
		{"RESULT_ER_DATA_TOO_LONG",umath::to_integral(::sql::Result::ErDataTooLong)},
		{"RESULT_ER_SP_BAD_SQLSTATE",umath::to_integral(::sql::Result::ErSpBadSqlstate)},
		{"RESULT_ER_STARTUP",umath::to_integral(::sql::Result::ErStartup)},
		{"RESULT_ER_LOAD_FROM_FIXED_SIZE_ROWS_TO_VAR",umath::to_integral(::sql::Result::ErLoadFromFixedSizeRowsToVar)},
		{"RESULT_ER_CANT_CREATE_USER_WITH_GRANT",umath::to_integral(::sql::Result::ErCantCreateUserWithGrant)},
		{"RESULT_ER_WRONG_VALUE_FOR_TYPE",umath::to_integral(::sql::Result::ErWrongValueForType)},
		{"RESULT_ER_TABLE_DEF_CHANGED",umath::to_integral(::sql::Result::ErTableDefChanged)},
		{"RESULT_ER_SP_DUP_HANDLER",umath::to_integral(::sql::Result::ErSpDupHandler)},
		{"RESULT_ER_SP_NOT_VAR_ARG",umath::to_integral(::sql::Result::ErSpNotVarArg)},
		{"RESULT_ER_SP_NO_RETSET",umath::to_integral(::sql::Result::ErSpNoRetset)},
		{"RESULT_ER_CANT_CREATE_GEOMETRY_OBJECT",umath::to_integral(::sql::Result::ErCantCreateGeometryObject)},
		{"RESULT_ER_FAILED_ROUTINE_BREAK_BINLOG",umath::to_integral(::sql::Result::ErFailedRoutineBreakBinlog)},
		{"RESULT_ER_BINLOG_UNSAFE_ROUTINE",umath::to_integral(::sql::Result::ErBinlogUnsafeRoutine)},
		{"RESULT_ER_BINLOG_CREATE_ROUTINE_NEED_SUPER",umath::to_integral(::sql::Result::ErBinlogCreateRoutineNeedSuper)},
		{"RESULT_ER_EXEC_STMT_WITH_OPEN_CURSOR",umath::to_integral(::sql::Result::ErExecStmtWithOpenCursor)},
		{"RESULT_ER_STMT_HAS_NO_OPEN_CURSOR",umath::to_integral(::sql::Result::ErStmtHasNoOpenCursor)},
		{"RESULT_ER_COMMIT_NOT_ALLOWED_IN_SF_OR_TRG",umath::to_integral(::sql::Result::ErCommitNotAllowedInSfOrTrg)},
		{"RESULT_ER_NO_DEFAULT_FOR_VIEW_FIELD",umath::to_integral(::sql::Result::ErNoDefaultForViewField)},
		{"RESULT_ER_SP_NO_RECURSION",umath::to_integral(::sql::Result::ErSpNoRecursion)},
		{"RESULT_ER_TOO_BIG_SCALE",umath::to_integral(::sql::Result::ErTooBigScale)},
		{"RESULT_ER_TOO_BIG_PRECISION",umath::to_integral(::sql::Result::ErTooBigPrecision)},
		{"RESULT_ER_M_BIGGER_THAN_D",umath::to_integral(::sql::Result::ErMBiggerThanD)},
		{"RESULT_ER_WRONG_LOCK_OF_SYSTEM_TABLE",umath::to_integral(::sql::Result::ErWrongLockOfSystemTable)},
		{"RESULT_ER_CONNECT_TO_FOREIGN_DATA_SOURCE",umath::to_integral(::sql::Result::ErConnectToForeignDataSource)},
		{"RESULT_ER_QUERY_ON_FOREIGN_DATA_SOURCE",umath::to_integral(::sql::Result::ErQueryOnForeignDataSource)},
		{"RESULT_ER_FOREIGN_DATA_SOURCE_DOESNT_EXIST",umath::to_integral(::sql::Result::ErForeignDataSourceDoesntExist)},
		{"RESULT_ER_FOREIGN_DATA_STRING_INVALID_CANT_CREATE",umath::to_integral(::sql::Result::ErForeignDataStringInvalidCantCreate)},
		{"RESULT_ER_FOREIGN_DATA_STRING_INVALID",umath::to_integral(::sql::Result::ErForeignDataStringInvalid)},
		{"RESULT_ER_CANT_CREATE_FEDERATED_TABLE",umath::to_integral(::sql::Result::ErCantCreateFederatedTable)},
		{"RESULT_ER_TRG_IN_WRONG_SCHEMA",umath::to_integral(::sql::Result::ErTrgInWrongSchema)},
		{"RESULT_ER_STACK_OVERRUN_NEED_MORE",umath::to_integral(::sql::Result::ErStackOverrunNeedMore)},
		{"RESULT_ER_TOO_LONG_BODY",umath::to_integral(::sql::Result::ErTooLongBody)},
		{"RESULT_ER_WARN_CANT_DROP_DEFAULT_KEYCACHE",umath::to_integral(::sql::Result::ErWarnCantDropDefaultKeycache)},
		{"RESULT_ER_TOO_BIG_DISPLAYWIDTH",umath::to_integral(::sql::Result::ErTooBigDisplaywidth)},
		{"RESULT_ER_XAER_DUPID",umath::to_integral(::sql::Result::ErXaerDupid)},
		{"RESULT_ER_DATETIME_FUNCTION_OVERFLOW",umath::to_integral(::sql::Result::ErDatetimeFunctionOverflow)},
		{"RESULT_ER_CANT_UPDATE_USED_TABLE_IN_SF_OR_TRG",umath::to_integral(::sql::Result::ErCantUpdateUsedTableInSfOrTrg)},
		{"RESULT_ER_VIEW_PREVENT_UPDATE",umath::to_integral(::sql::Result::ErViewPreventUpdate)},
		{"RESULT_ER_PS_NO_RECURSION",umath::to_integral(::sql::Result::ErPsNoRecursion)},
		{"RESULT_ER_SP_CANT_SET_AUTOCOMMIT",umath::to_integral(::sql::Result::ErSpCantSetAutocommit)},
		{"RESULT_ER_MALFORMED_DEFINER",umath::to_integral(::sql::Result::ErMalformedDefiner)},
		{"RESULT_ER_VIEW_FRM_NO_USER",umath::to_integral(::sql::Result::ErViewFrmNoUser)},
		{"RESULT_ER_VIEW_OTHER_USER",umath::to_integral(::sql::Result::ErViewOtherUser)},
		{"RESULT_ER_NO_SUCH_USER",umath::to_integral(::sql::Result::ErNoSuchUser)},
		{"RESULT_ER_FORBID_SCHEMA_CHANGE",umath::to_integral(::sql::Result::ErForbidSchemaChange)},
		{"RESULT_ER_ROW_IS_REFERENCED_2",umath::to_integral(::sql::Result::ErRowIsReferenced2)},
		{"RESULT_ER_NO_REFERENCED_ROW_2",umath::to_integral(::sql::Result::ErNoReferencedRow2)},
		{"RESULT_ER_SP_BAD_VAR_SHADOW",umath::to_integral(::sql::Result::ErSpBadVarShadow)},
		{"RESULT_ER_TRG_NO_DEFINER",umath::to_integral(::sql::Result::ErTrgNoDefiner)},
		{"RESULT_ER_OLD_FILE_FORMAT",umath::to_integral(::sql::Result::ErOldFileFormat)},
		{"RESULT_ER_SP_RECURSION_LIMIT",umath::to_integral(::sql::Result::ErSpRecursionLimit)},
		{"RESULT_ER_SP_PROC_TABLE_CORRUPT",umath::to_integral(::sql::Result::ErSpProcTableCorrupt)},
		{"RESULT_ER_SP_WRONG_NAME",umath::to_integral(::sql::Result::ErSpWrongName)},
		{"RESULT_ER_TABLE_NEEDS_UPGRADE",umath::to_integral(::sql::Result::ErTableNeedsUpgrade)},
		{"RESULT_ER_SP_NO_AGGREGATE",umath::to_integral(::sql::Result::ErSpNoAggregate)},
		{"RESULT_ER_MAX_PREPARED_STMT_COUNT_REACHED",umath::to_integral(::sql::Result::ErMaxPreparedStmtCountReached)},
		{"RESULT_ER_VIEW_RECURSIVE",umath::to_integral(::sql::Result::ErViewRecursive)},
		{"RESULT_ER_NON_GROUPING_FIELD_USED",umath::to_integral(::sql::Result::ErNonGroupingFieldUsed)},
		{"RESULT_ER_TABLE_CANT_HANDLE_SPKEYS",umath::to_integral(::sql::Result::ErTableCantHandleSpkeys)},
		{"RESULT_ER_NO_TRIGGERS_ON_SYSTEM_SCHEMA",umath::to_integral(::sql::Result::ErNoTriggersOnSystemSchema)},
		{"RESULT_ER_REMOVED_SPACES",umath::to_integral(::sql::Result::ErRemovedSpaces)},
		{"RESULT_ER_AUTOINC_READ_FAILED",umath::to_integral(::sql::Result::ErAutoincReadFailed)},
		{"RESULT_ER_USERNAME",umath::to_integral(::sql::Result::ErUsername)},
		{"RESULT_ER_HOSTNAME",umath::to_integral(::sql::Result::ErHostname)},
		{"RESULT_ER_WRONG_STRING_LENGTH",umath::to_integral(::sql::Result::ErWrongStringLength)},
		{"RESULT_ER_NON_INSERTABLE_TABLE",umath::to_integral(::sql::Result::ErNonInsertableTable)},
		{"RESULT_ER_ADMIN_WRONG_MRG_TABLE",umath::to_integral(::sql::Result::ErAdminWrongMrgTable)},
		{"RESULT_ER_TOO_HIGH_LEVEL_OF_NESTING_FOR_SELECT",umath::to_integral(::sql::Result::ErTooHighLevelOfNestingForSelect)},
		{"RESULT_ER_NAME_BECOMES_EMPTY",umath::to_integral(::sql::Result::ErNameBecomesEmpty)},
		{"RESULT_ER_AMBIGUOUS_FIELD_TERM",umath::to_integral(::sql::Result::ErAmbiguousFieldTerm)},
		{"RESULT_ER_FOREIGN_SERVER_EXISTS",umath::to_integral(::sql::Result::ErForeignServerExists)},
		{"RESULT_ER_FOREIGN_SERVER_DOESNT_EXIST",umath::to_integral(::sql::Result::ErForeignServerDoesntExist)},
		{"RESULT_ER_ILLEGAL_HA_CREATE_OPTION",umath::to_integral(::sql::Result::ErIllegalHaCreateOption)},
		{"RESULT_ER_PARTITION_REQUIRES_VALUES_ERROR",umath::to_integral(::sql::Result::ErPartitionRequiresValuesError)},
		{"RESULT_ER_PARTITION_WRONG_VALUES_ERROR",umath::to_integral(::sql::Result::ErPartitionWrongValuesError)},
		{"RESULT_ER_PARTITION_MAXVALUE_ERROR",umath::to_integral(::sql::Result::ErPartitionMaxvalueError)},
		{"RESULT_ER_PARTITION_SUBPARTITION_ERROR",umath::to_integral(::sql::Result::ErPartitionSubpartitionError)},
		{"RESULT_ER_PARTITION_SUBPART_MIX_ERROR",umath::to_integral(::sql::Result::ErPartitionSubpartMixError)},
		{"RESULT_ER_PARTITION_WRONG_NO_PART_ERROR",umath::to_integral(::sql::Result::ErPartitionWrongNoPartError)},
		{"RESULT_ER_PARTITION_WRONG_NO_SUBPART_ERROR",umath::to_integral(::sql::Result::ErPartitionWrongNoSubpartError)},
		{"RESULT_ER_CONST_EXPR_IN_PARTITION_FUNC_ERROR",umath::to_integral(::sql::Result::ErConstExprInPartitionFuncError)},
		{"RESULT_ER_NO_CONST_EXPR_IN_RANGE_OR_LIST_ERROR",umath::to_integral(::sql::Result::ErNoConstExprInRangeOrListError)},
		{"RESULT_ER_FIELD_NOT_FOUND_PART_ERROR",umath::to_integral(::sql::Result::ErFieldNotFoundPartError)},
		{"RESULT_ER_LIST_OF_FIELDS_ONLY_IN_HASH_ERROR",umath::to_integral(::sql::Result::ErListOfFieldsOnlyInHashError)},
		{"RESULT_ER_INCONSISTENT_PARTITION_INFO_ERROR",umath::to_integral(::sql::Result::ErInconsistentPartitionInfoError)},
		{"RESULT_ER_PARTITION_FUNC_NOT_ALLOWED_ERROR",umath::to_integral(::sql::Result::ErPartitionFuncNotAllowedError)},
		{"RESULT_ER_PARTITIONS_MUST_BE_DEFINED_ERROR",umath::to_integral(::sql::Result::ErPartitionsMustBeDefinedError)},
		{"RESULT_ER_RANGE_NOT_INCREASING_ERROR",umath::to_integral(::sql::Result::ErRangeNotIncreasingError)},
		{"RESULT_ER_INCONSISTENT_TYPE_OF_FUNCTIONS_ERROR",umath::to_integral(::sql::Result::ErInconsistentTypeOfFunctionsError)},
		{"RESULT_ER_MULTIPLE_DEF_CONST_IN_LIST_PART_ERROR",umath::to_integral(::sql::Result::ErMultipleDefConstInListPartError)},
		{"RESULT_ER_PARTITION_ENTRY_ERROR",umath::to_integral(::sql::Result::ErPartitionEntryError)},
		{"RESULT_ER_MIX_HANDLER_ERROR",umath::to_integral(::sql::Result::ErMixHandlerError)},
		{"RESULT_ER_PARTITION_NOT_DEFINED_ERROR",umath::to_integral(::sql::Result::ErPartitionNotDefinedError)},
		{"RESULT_ER_TOO_MANY_PARTITIONS_ERROR",umath::to_integral(::sql::Result::ErTooManyPartitionsError)},
		{"RESULT_ER_SUBPARTITION_ERROR",umath::to_integral(::sql::Result::ErSubpartitionError)},
		{"RESULT_ER_CANT_CREATE_HANDLER_FILE",umath::to_integral(::sql::Result::ErCantCreateHandlerFile)},
		{"RESULT_ER_BLOB_FIELD_IN_PART_FUNC_ERROR",umath::to_integral(::sql::Result::ErBlobFieldInPartFuncError)},
		{"RESULT_ER_UNIQUE_KEY_NEED_ALL_FIELDS_IN_PF",umath::to_integral(::sql::Result::ErUniqueKeyNeedAllFieldsInPf)},
		{"RESULT_ER_NO_PARTS_ERROR",umath::to_integral(::sql::Result::ErNoPartsError)},
		{"RESULT_ER_PARTITION_MGMT_ON_NONPARTITIONED",umath::to_integral(::sql::Result::ErPartitionMgmtOnNonpartitioned)},
		{"RESULT_ER_FOREIGN_KEY_ON_PARTITIONED",umath::to_integral(::sql::Result::ErForeignKeyOnPartitioned)},
		{"RESULT_ER_DROP_PARTITION_NON_EXISTENT",umath::to_integral(::sql::Result::ErDropPartitionNonExistent)},
		{"RESULT_ER_DROP_LAST_PARTITION",umath::to_integral(::sql::Result::ErDropLastPartition)},
		{"RESULT_ER_COALESCE_ONLY_ON_HASH_PARTITION",umath::to_integral(::sql::Result::ErCoalesceOnlyOnHashPartition)},
		{"RESULT_ER_REORG_HASH_ONLY_ON_SAME_NO",umath::to_integral(::sql::Result::ErReorgHashOnlyOnSameNo)},
		{"RESULT_ER_REORG_NO_PARAM_ERROR",umath::to_integral(::sql::Result::ErReorgNoParamError)},
		{"RESULT_ER_ONLY_ON_RANGE_LIST_PARTITION",umath::to_integral(::sql::Result::ErOnlyOnRangeListPartition)},
		{"RESULT_ER_ADD_PARTITION_SUBPART_ERROR",umath::to_integral(::sql::Result::ErAddPartitionSubpartError)},
		{"RESULT_ER_ADD_PARTITION_NO_NEW_PARTITION",umath::to_integral(::sql::Result::ErAddPartitionNoNewPartition)},
		{"RESULT_ER_COALESCE_PARTITION_NO_PARTITION",umath::to_integral(::sql::Result::ErCoalescePartitionNoPartition)},
		{"RESULT_ER_REORG_PARTITION_NOT_EXIST",umath::to_integral(::sql::Result::ErReorgPartitionNotExist)},
		{"RESULT_ER_SAME_NAME_PARTITION",umath::to_integral(::sql::Result::ErSameNamePartition)},
		{"RESULT_ER_NO_BINLOG_ERROR",umath::to_integral(::sql::Result::ErNoBinlogError)},
		{"RESULT_ER_CONSECUTIVE_REORG_PARTITIONS",umath::to_integral(::sql::Result::ErConsecutiveReorgPartitions)},
		{"RESULT_ER_REORG_OUTSIDE_RANGE",umath::to_integral(::sql::Result::ErReorgOutsideRange)},
		{"RESULT_ER_PARTITION_FUNCTION_FAILURE",umath::to_integral(::sql::Result::ErPartitionFunctionFailure)},
		{"RESULT_ER_PART_STATE_ERROR",umath::to_integral(::sql::Result::ErPartStateError)},
		{"RESULT_ER_LIMITED_PART_RANGE",umath::to_integral(::sql::Result::ErLimitedPartRange)},
		{"RESULT_ER_PLUGIN_IS_NOT_LOADED",umath::to_integral(::sql::Result::ErPluginIsNotLoaded)},
		{"RESULT_ER_WRONG_VALUE",umath::to_integral(::sql::Result::ErWrongValue)},
		{"RESULT_ER_NO_PARTITION_FOR_GIVEN_VALUE",umath::to_integral(::sql::Result::ErNoPartitionForGivenValue)},
		{"RESULT_ER_FILEGROUP_OPTION_ONLY_ONCE",umath::to_integral(::sql::Result::ErFilegroupOptionOnlyOnce)},
		{"RESULT_ER_CREATE_FILEGROUP_FAILED",umath::to_integral(::sql::Result::ErCreateFilegroupFailed)},
		{"RESULT_ER_DROP_FILEGROUP_FAILED",umath::to_integral(::sql::Result::ErDropFilegroupFailed)},
		{"RESULT_ER_TABLESPACE_AUTO_EXTEND_ERROR",umath::to_integral(::sql::Result::ErTablespaceAutoExtendError)},
		{"RESULT_ER_WRONG_SIZE_NUMBER",umath::to_integral(::sql::Result::ErWrongSizeNumber)},
		{"RESULT_ER_SIZE_OVERFLOW_ERROR",umath::to_integral(::sql::Result::ErSizeOverflowError)},
		{"RESULT_ER_ALTER_FILEGROUP_FAILED",umath::to_integral(::sql::Result::ErAlterFilegroupFailed)},
		{"RESULT_ER_BINLOG_ROW_LOGGING_FAILED",umath::to_integral(::sql::Result::ErBinlogRowLoggingFailed)},
		{"RESULT_ER_BINLOG_ROW_WRONG_TABLE_DEF",umath::to_integral(::sql::Result::ErBinlogRowWrongTableDef)},
		{"RESULT_ER_BINLOG_ROW_RBR_TO_SBR",umath::to_integral(::sql::Result::ErBinlogRowRbrToSbr)},
		{"RESULT_ER_EVENT_ALREADY_EXISTS",umath::to_integral(::sql::Result::ErEventAlreadyExists)},
		{"RESULT_ER_EVENT_STORE_FAILED",umath::to_integral(::sql::Result::ErEventStoreFailed)},
		{"RESULT_ER_EVENT_DOES_NOT_EXIST",umath::to_integral(::sql::Result::ErEventDoesNotExist)},
		{"RESULT_ER_EVENT_CANT_ALTER",umath::to_integral(::sql::Result::ErEventCantAlter)},
		{"RESULT_ER_EVENT_DROP_FAILED",umath::to_integral(::sql::Result::ErEventDropFailed)},
		{"RESULT_ER_EVENT_INTERVAL_NOT_POSITIVE_OR_TOO_BIG",umath::to_integral(::sql::Result::ErEventIntervalNotPositiveOrTooBig)},
		{"RESULT_ER_EVENT_ENDS_BEFORE_STARTS",umath::to_integral(::sql::Result::ErEventEndsBeforeStarts)},
		{"RESULT_ER_EVENT_EXEC_TIME_IN_THE_PAST",umath::to_integral(::sql::Result::ErEventExecTimeInThePast)},
		{"RESULT_ER_EVENT_OPEN_TABLE_FAILED",umath::to_integral(::sql::Result::ErEventOpenTableFailed)},
		{"RESULT_ER_EVENT_NEITHER_M_EXPR_NOR_M_AT",umath::to_integral(::sql::Result::ErEventNeitherMExprNorMAt)},
		{"RESULT_ER_COL_COUNT_DOESNT_MATCH_CORRUPTED",umath::to_integral(::sql::Result::ErColCountDoesntMatchCorrupted)},
		{"RESULT_ER_CANNOT_LOAD_FROM_TABLE",umath::to_integral(::sql::Result::ErCannotLoadFromTable)},
		{"RESULT_ER_EVENT_CANNOT_DELETE",umath::to_integral(::sql::Result::ErEventCannotDelete)},
		{"RESULT_ER_EVENT_COMPILE_ERROR",umath::to_integral(::sql::Result::ErEventCompileError)},
		{"RESULT_ER_EVENT_SAME_NAME",umath::to_integral(::sql::Result::ErEventSameName)},
		{"RESULT_ER_EVENT_DATA_TOO_LONG",umath::to_integral(::sql::Result::ErEventDataTooLong)},
		{"RESULT_ER_DROP_INDEX_FK",umath::to_integral(::sql::Result::ErDropIndexFk)},
		{"RESULT_ER_WARN_DEPRECATED_SYNTAX_WITH_VER",umath::to_integral(::sql::Result::ErWarnDeprecatedSyntaxWithVer)},
		{"RESULT_ER_CANT_WRITE_LOCK_LOG_TABLE",umath::to_integral(::sql::Result::ErCantWriteLockLogTable)},
		{"RESULT_ER_CANT_LOCK_LOG_TABLE",umath::to_integral(::sql::Result::ErCantLockLogTable)},
		{"RESULT_ER_FOREIGN_DUPLICATE_KEY",umath::to_integral(::sql::Result::ErForeignDuplicateKey)},
		{"RESULT_ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE",umath::to_integral(::sql::Result::ErColCountDoesntMatchPleaseUpdate)},
		{"RESULT_ER_TEMP_TABLE_PREVENTS_SWITCH_OUT_OF_RBR",umath::to_integral(::sql::Result::ErTempTablePreventsSwitchOutOfRbr)},
		{"RESULT_ER_STORED_FUNCTION_PREVENTS_SWITCH_BINLOG_FORMAT",umath::to_integral(::sql::Result::ErStoredFunctionPreventsSwitchBinlogFormat)},
		{"RESULT_ER_NDB_CANT_SWITCH_BINLOG_FORMAT",umath::to_integral(::sql::Result::ErNdbCantSwitchBinlogFormat)},
		{"RESULT_ER_PARTITION_NO_TEMPORARY",umath::to_integral(::sql::Result::ErPartitionNoTemporary)},
		{"RESULT_ER_PARTITION_CONST_DOMAIN_ERROR",umath::to_integral(::sql::Result::ErPartitionConstDomainError)},
		{"RESULT_ER_PARTITION_FUNCTION_IS_NOT_ALLOWED",umath::to_integral(::sql::Result::ErPartitionFunctionIsNotAllowed)},
		{"RESULT_ER_DDL_LOG_ERROR",umath::to_integral(::sql::Result::ErDdlLogError)},
		{"RESULT_ER_NULL_IN_VALUES_LESS_THAN",umath::to_integral(::sql::Result::ErNullInValuesLessThan)},
		{"RESULT_ER_WRONG_PARTITION_NAME",umath::to_integral(::sql::Result::ErWrongPartitionName)},
		{"RESULT_ER_CANT_CHANGE_TX_ISOLATION",umath::to_integral(::sql::Result::ErCantChangeTxIsolation)},
		{"RESULT_ER_DUP_ENTRY_AUTOINCREMENT_CASE",umath::to_integral(::sql::Result::ErDupEntryAutoincrementCase)},
		{"RESULT_ER_EVENT_MODIFY_QUEUE_ERROR",umath::to_integral(::sql::Result::ErEventModifyQueueError)},
		{"RESULT_ER_EVENT_SET_VAR_ERROR",umath::to_integral(::sql::Result::ErEventSetVarError)},
		{"RESULT_ER_PARTITION_MERGE_ERROR",umath::to_integral(::sql::Result::ErPartitionMergeError)},
		{"RESULT_ER_CANT_ACTIVATE_LOG",umath::to_integral(::sql::Result::ErCantActivateLog)},
		{"RESULT_ER_RBR_NOT_AVAILABLE",umath::to_integral(::sql::Result::ErRbrNotAvailable)},
		{"RESULT_ER_BASE64_DECODE_ERROR",umath::to_integral(::sql::Result::ErBase64DecodeError)},
		{"RESULT_ER_EVENT_RECURSION_FORBIDDEN",umath::to_integral(::sql::Result::ErEventRecursionForbidden)},
		{"RESULT_ER_EVENTS_DB_ERROR",umath::to_integral(::sql::Result::ErEventsDbError)},
		{"RESULT_ER_ONLY_INTEGERS_ALLOWED",umath::to_integral(::sql::Result::ErOnlyIntegersAllowed)},
		{"RESULT_ER_UNSUPORTED_LOG_ENGINE",umath::to_integral(::sql::Result::ErUnsuportedLogEngine)},
		{"RESULT_ER_BAD_LOG_STATEMENT",umath::to_integral(::sql::Result::ErBadLogStatement)},
		{"RESULT_ER_CANT_RENAME_LOG_TABLE",umath::to_integral(::sql::Result::ErCantRenameLogTable)},
		{"RESULT_ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT",umath::to_integral(::sql::Result::ErWrongParamcountToNativeFct)},
		{"RESULT_ER_WRONG_PARAMETERS_TO_NATIVE_FCT",umath::to_integral(::sql::Result::ErWrongParametersToNativeFct)},
		{"RESULT_ER_WRONG_PARAMETERS_TO_STORED_FCT",umath::to_integral(::sql::Result::ErWrongParametersToStoredFct)},
		{"RESULT_ER_NATIVE_FCT_NAME_COLLISION",umath::to_integral(::sql::Result::ErNativeFctNameCollision)},
		{"RESULT_ER_DUP_ENTRY_WITH_KEY_NAME",umath::to_integral(::sql::Result::ErDupEntryWithKeyName)},
		{"RESULT_ER_BINLOG_PURGE_EMFILE",umath::to_integral(::sql::Result::ErBinlogPurgeEmfile)},
		{"RESULT_ER_EVENT_CANNOT_CREATE_IN_THE_PAST",umath::to_integral(::sql::Result::ErEventCannotCreateInThePast)},
		{"RESULT_ER_EVENT_CANNOT_ALTER_IN_THE_PAST",umath::to_integral(::sql::Result::ErEventCannotAlterInThePast)},
		{"RESULT_ER_SLAVE_INCIDENT",umath::to_integral(::sql::Result::ErSlaveIncident)},
		{"RESULT_ER_NO_PARTITION_FOR_GIVEN_VALUE_SILENT",umath::to_integral(::sql::Result::ErNoPartitionForGivenValueSilent)},
		{"RESULT_ER_BINLOG_UNSAFE_STATEMENT",umath::to_integral(::sql::Result::ErBinlogUnsafeStatement)},
		{"RESULT_ER_SLAVE_FATAL_ERROR",umath::to_integral(::sql::Result::ErSlaveFatalError)},
		{"RESULT_ER_SLAVE_RELAY_LOG_READ_FAILURE",umath::to_integral(::sql::Result::ErSlaveRelayLogReadFailure)},
		{"RESULT_ER_SLAVE_RELAY_LOG_WRITE_FAILURE",umath::to_integral(::sql::Result::ErSlaveRelayLogWriteFailure)},
		{"RESULT_ER_SLAVE_CREATE_EVENT_FAILURE",umath::to_integral(::sql::Result::ErSlaveCreateEventFailure)},
		{"RESULT_ER_SLAVE_MASTER_COM_FAILURE",umath::to_integral(::sql::Result::ErSlaveMasterComFailure)},
		{"RESULT_ER_BINLOG_LOGGING_IMPOSSIBLE",umath::to_integral(::sql::Result::ErBinlogLoggingImpossible)},
		{"RESULT_ER_VIEW_NO_CREATION_CTX",umath::to_integral(::sql::Result::ErViewNoCreationCtx)},
		{"RESULT_ER_VIEW_INVALID_CREATION_CTX",umath::to_integral(::sql::Result::ErViewInvalidCreationCtx)},
		{"RESULT_ER_SR_INVALID_CREATION_CTX",umath::to_integral(::sql::Result::ErSrInvalidCreationCtx)},
		{"RESULT_ER_TRG_CORRUPTED_FILE",umath::to_integral(::sql::Result::ErTrgCorruptedFile)},
		{"RESULT_ER_TRG_NO_CREATION_CTX",umath::to_integral(::sql::Result::ErTrgNoCreationCtx)},
		{"RESULT_ER_TRG_INVALID_CREATION_CTX",umath::to_integral(::sql::Result::ErTrgInvalidCreationCtx)},
		{"RESULT_ER_EVENT_INVALID_CREATION_CTX",umath::to_integral(::sql::Result::ErEventInvalidCreationCtx)},
		{"RESULT_ER_TRG_CANT_OPEN_TABLE",umath::to_integral(::sql::Result::ErTrgCantOpenTable)},
		{"RESULT_ER_CANT_CREATE_SROUTINE",umath::to_integral(::sql::Result::ErCantCreateSroutine)},
		{"RESULT_ER_SLAVE_AMBIGOUS_EXEC_MODE",umath::to_integral(::sql::Result::ErSlaveAmbigousExecMode)},
		{"RESULT_ER_NO_FORMAT_DESCRIPTION_EVENT_BEFORE_BINLOG_STATEMENT",umath::to_integral(::sql::Result::ErNoFormatDescriptionEventBeforeBinlogStatement)},
		{"RESULT_ER_SLAVE_CORRUPT_EVENT",umath::to_integral(::sql::Result::ErSlaveCorruptEvent)},
		{"RESULT_ER_LOAD_DATA_INVALID_COLUMN",umath::to_integral(::sql::Result::ErLoadDataInvalidColumn)},
		{"RESULT_ER_LOG_PURGE_NO_FILE",umath::to_integral(::sql::Result::ErLogPurgeNoFile)},
		{"RESULT_ER_NEED_REPREPARE",umath::to_integral(::sql::Result::ErNeedReprepare)},
		{"RESULT_ER_DELAYED_NOT_SUPPORTED",umath::to_integral(::sql::Result::ErDelayedNotSupported)},
		{"RESULT_ER_WARN_AUTO_CONVERT_LOCK",umath::to_integral(::sql::Result::ErWarnAutoConvertLock)},
		{"RESULT_ER_NO_AUTO_CONVERT_LOCK_STRICT",umath::to_integral(::sql::Result::ErNoAutoConvertLockStrict)},
		{"RESULT_ER_NO_AUTO_CONVERT_LOCK_TRANSACTION",umath::to_integral(::sql::Result::ErNoAutoConvertLockTransaction)},
		{"RESULT_ER_NO_STORAGE_ENGINE",umath::to_integral(::sql::Result::ErNoStorageEngine)},
		{"RESULT_ER_BACKUP_BACKUP_START",umath::to_integral(::sql::Result::ErBackupBackupStart)},
		{"RESULT_ER_BACKUP_BACKUP_DONE",umath::to_integral(::sql::Result::ErBackupBackupDone)},
		{"RESULT_ER_BACKUP_RESTORE_START",umath::to_integral(::sql::Result::ErBackupRestoreStart)},
		{"RESULT_ER_BACKUP_RESTORE_DONE",umath::to_integral(::sql::Result::ErBackupRestoreDone)},
		{"RESULT_ER_BACKUP_NOTHING_TO_BACKUP",umath::to_integral(::sql::Result::ErBackupNothingToBackup)},
		{"RESULT_ER_BACKUP_CANNOT_INCLUDE_DB",umath::to_integral(::sql::Result::ErBackupCannotIncludeDb)},
		{"RESULT_ER_BACKUP_BACKUP",umath::to_integral(::sql::Result::ErBackupBackup)},
		{"RESULT_ER_BACKUP_RESTORE",umath::to_integral(::sql::Result::ErBackupRestore)},
		{"RESULT_ER_BACKUP_RUNNING",umath::to_integral(::sql::Result::ErBackupRunning)},
		{"RESULT_ER_BACKUP_BACKUP_PREPARE",umath::to_integral(::sql::Result::ErBackupBackupPrepare)},
		{"RESULT_ER_BACKUP_RESTORE_PREPARE",umath::to_integral(::sql::Result::ErBackupRestorePrepare)},
		{"RESULT_ER_BACKUP_INVALID_LOC",umath::to_integral(::sql::Result::ErBackupInvalidLoc)},
		{"RESULT_ER_BACKUP_READ_LOC",umath::to_integral(::sql::Result::ErBackupReadLoc)},
		{"RESULT_ER_BACKUP_WRITE_LOC",umath::to_integral(::sql::Result::ErBackupWriteLoc)},
		{"RESULT_ER_BACKUP_LIST_DBS",umath::to_integral(::sql::Result::ErBackupListDbs)},
		{"RESULT_ER_BACKUP_LIST_TABLES",umath::to_integral(::sql::Result::ErBackupListTables)},
		{"RESULT_ER_BACKUP_LIST_DB_TABLES",umath::to_integral(::sql::Result::ErBackupListDbTables)},
		{"RESULT_ER_BACKUP_SKIP_VIEW",umath::to_integral(::sql::Result::ErBackupSkipView)},
		{"RESULT_ER_BACKUP_NO_ENGINE",umath::to_integral(::sql::Result::ErBackupNoEngine)},
		{"RESULT_ER_BACKUP_TABLE_OPEN",umath::to_integral(::sql::Result::ErBackupTableOpen)},
		{"RESULT_ER_BACKUP_READ_HEADER",umath::to_integral(::sql::Result::ErBackupReadHeader)},
		{"RESULT_ER_BACKUP_WRITE_HEADER",umath::to_integral(::sql::Result::ErBackupWriteHeader)},
		{"RESULT_ER_BACKUP_NO_BACKUP_DRIVER",umath::to_integral(::sql::Result::ErBackupNoBackupDriver)},
		{"RESULT_ER_BACKUP_NOT_ACCEPTED",umath::to_integral(::sql::Result::ErBackupNotAccepted)},
		{"RESULT_ER_BACKUP_CREATE_BACKUP_DRIVER",umath::to_integral(::sql::Result::ErBackupCreateBackupDriver)},
		{"RESULT_ER_BACKUP_CREATE_RESTORE_DRIVER",umath::to_integral(::sql::Result::ErBackupCreateRestoreDriver)},
		{"RESULT_ER_BACKUP_TOO_MANY_IMAGES",umath::to_integral(::sql::Result::ErBackupTooManyImages)},
		{"RESULT_ER_BACKUP_WRITE_META",umath::to_integral(::sql::Result::ErBackupWriteMeta)},
		{"RESULT_ER_BACKUP_READ_META",umath::to_integral(::sql::Result::ErBackupReadMeta)},
		{"RESULT_ER_BACKUP_CREATE_META",umath::to_integral(::sql::Result::ErBackupCreateMeta)},
		{"RESULT_ER_BACKUP_GET_BUF",umath::to_integral(::sql::Result::ErBackupGetBuf)},
		{"RESULT_ER_BACKUP_WRITE_DATA",umath::to_integral(::sql::Result::ErBackupWriteData)},
		{"RESULT_ER_BACKUP_READ_DATA",umath::to_integral(::sql::Result::ErBackupReadData)},
		{"RESULT_ER_BACKUP_NEXT_CHUNK",umath::to_integral(::sql::Result::ErBackupNextChunk)},
		{"RESULT_ER_BACKUP_INIT_BACKUP_DRIVER",umath::to_integral(::sql::Result::ErBackupInitBackupDriver)},
		{"RESULT_ER_BACKUP_INIT_RESTORE_DRIVER",umath::to_integral(::sql::Result::ErBackupInitRestoreDriver)},
		{"RESULT_ER_BACKUP_STOP_BACKUP_DRIVER",umath::to_integral(::sql::Result::ErBackupStopBackupDriver)},
		{"RESULT_ER_BACKUP_STOP_RESTORE_DRIVERS",umath::to_integral(::sql::Result::ErBackupStopRestoreDrivers)},
		{"RESULT_ER_BACKUP_PREPARE_DRIVER",umath::to_integral(::sql::Result::ErBackupPrepareDriver)},
		{"RESULT_ER_BACKUP_CREATE_VP",umath::to_integral(::sql::Result::ErBackupCreateVp)},
		{"RESULT_ER_BACKUP_UNLOCK_DRIVER",umath::to_integral(::sql::Result::ErBackupUnlockDriver)},
		{"RESULT_ER_BACKUP_CANCEL_BACKUP",umath::to_integral(::sql::Result::ErBackupCancelBackup)},
		{"RESULT_ER_BACKUP_CANCEL_RESTORE",umath::to_integral(::sql::Result::ErBackupCancelRestore)},
		{"RESULT_ER_BACKUP_GET_DATA",umath::to_integral(::sql::Result::ErBackupGetData)},
		{"RESULT_ER_BACKUP_SEND_DATA",umath::to_integral(::sql::Result::ErBackupSendData)},
		{"RESULT_ER_BACKUP_SEND_DATA_RETRY",umath::to_integral(::sql::Result::ErBackupSendDataRetry)},
		{"RESULT_ER_BACKUP_OPEN_TABLES",umath::to_integral(::sql::Result::ErBackupOpenTables)},
		{"RESULT_ER_BACKUP_THREAD_INIT",umath::to_integral(::sql::Result::ErBackupThreadInit)},
		{"RESULT_ER_BACKUP_PROGRESS_TABLES",umath::to_integral(::sql::Result::ErBackupProgressTables)},
		{"RESULT_ER_TABLESPACE_EXIST",umath::to_integral(::sql::Result::ErTablespaceExist)},
		{"RESULT_ER_NO_SUCH_TABLESPACE",umath::to_integral(::sql::Result::ErNoSuchTablespace)},
		{"RESULT_ER_SLAVE_HEARTBEAT_FAILURE",umath::to_integral(::sql::Result::ErSlaveHeartbeatFailure)},
		{"RESULT_ER_SLAVE_HEARTBEAT_VALUE_OUT_OF_RANGE",umath::to_integral(::sql::Result::ErSlaveHeartbeatValueOutOfRange)},
		{"RESULT_ER_BACKUP_CANT_FIND_SE",umath::to_integral(::sql::Result::ErBackupCantFindSe)},
		{"RESULT_ER_BACKUP_NO_NATIVE_BE",umath::to_integral(::sql::Result::ErBackupNoNativeBe)},
		{"RESULT_ER_BACKUP_UNKNOWN_BE",umath::to_integral(::sql::Result::ErBackupUnknownBe)},
		{"RESULT_ER_BACKUP_WRONG_TABLE_BE",umath::to_integral(::sql::Result::ErBackupWrongTableBe)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_DB",umath::to_integral(::sql::Result::ErBackupCantRestoreDb)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_TABLE",umath::to_integral(::sql::Result::ErBackupCantRestoreTable)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_VIEW",umath::to_integral(::sql::Result::ErBackupCantRestoreView)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_SROUT",umath::to_integral(::sql::Result::ErBackupCantRestoreSrout)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_EVENT",umath::to_integral(::sql::Result::ErBackupCantRestoreEvent)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_TRIGGER",umath::to_integral(::sql::Result::ErBackupCantRestoreTrigger)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_DB",umath::to_integral(::sql::Result::ErBackupCatalogAddDb)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_TABLE",umath::to_integral(::sql::Result::ErBackupCatalogAddTable)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_VIEW",umath::to_integral(::sql::Result::ErBackupCatalogAddView)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_SROUT",umath::to_integral(::sql::Result::ErBackupCatalogAddSrout)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_EVENT",umath::to_integral(::sql::Result::ErBackupCatalogAddEvent)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_TRIGGER",umath::to_integral(::sql::Result::ErBackupCatalogAddTrigger)},
		{"RESULT_ER_BACKUP_UNKNOWN_OBJECT",umath::to_integral(::sql::Result::ErBackupUnknownObject)},
		{"RESULT_ER_BACKUP_UNKNOWN_OBJECT_TYPE",umath::to_integral(::sql::Result::ErBackupUnknownObjectType)},
		{"RESULT_ER_BACKUP_OPEN_WR",umath::to_integral(::sql::Result::ErBackupOpenWr)},
		{"RESULT_ER_BACKUP_OPEN_RD",umath::to_integral(::sql::Result::ErBackupOpenRd)},
		{"RESULT_ER_BACKUP_BAD_MAGIC",umath::to_integral(::sql::Result::ErBackupBadMagic)},
		{"RESULT_ER_BACKUP_CONTEXT_CREATE",umath::to_integral(::sql::Result::ErBackupContextCreate)},
		{"RESULT_ER_BACKUP_CONTEXT_REMOVE",umath::to_integral(::sql::Result::ErBackupContextRemove)},
		{"RESULT_ER_BAD_PATH",umath::to_integral(::sql::Result::ErBadPath)},
		{"RESULT_ER_DDL_BLOCK",umath::to_integral(::sql::Result::ErDdlBlock)},
		{"RESULT_ER_BACKUP_LOGGER_INIT",umath::to_integral(::sql::Result::ErBackupLoggerInit)},
		{"RESULT_ER_BACKUP_WRITE_SUMMARY",umath::to_integral(::sql::Result::ErBackupWriteSummary)},
		{"RESULT_ER_BACKUP_READ_SUMMARY",umath::to_integral(::sql::Result::ErBackupReadSummary)},
		{"RESULT_ER_BACKUP_GET_META_DB",umath::to_integral(::sql::Result::ErBackupGetMetaDb)},
		{"RESULT_ER_BACKUP_GET_META_TABLE",umath::to_integral(::sql::Result::ErBackupGetMetaTable)},
		{"RESULT_ER_BACKUP_GET_META_VIEW",umath::to_integral(::sql::Result::ErBackupGetMetaView)},
		{"RESULT_ER_BACKUP_GET_META_SROUT",umath::to_integral(::sql::Result::ErBackupGetMetaSrout)},
		{"RESULT_ER_BACKUP_GET_META_EVENT",umath::to_integral(::sql::Result::ErBackupGetMetaEvent)},
		{"RESULT_ER_BACKUP_GET_META_TRIGGER",umath::to_integral(::sql::Result::ErBackupGetMetaTrigger)},
		{"RESULT_ER_BACKUP_CREATE_BE",umath::to_integral(::sql::Result::ErBackupCreateBe)},
		{"RESULT_ER_BACKUP_LIST_PERDB",umath::to_integral(::sql::Result::ErBackupListPerdb)},
		{"RESULT_ER_BACKUP_LIST_DB_VIEWS",umath::to_integral(::sql::Result::ErBackupListDbViews)},
		{"RESULT_ER_BACKUP_LIST_DB_SROUT",umath::to_integral(::sql::Result::ErBackupListDbSrout)},
		{"RESULT_ER_BACKUP_LIST_DB_EVENTS",umath::to_integral(::sql::Result::ErBackupListDbEvents)},
		{"RESULT_ER_BACKUP_LIST_DB_TRIGGERS",umath::to_integral(::sql::Result::ErBackupListDbTriggers)},
		{"RESULT_ER_BACKUP_LOG_WRITE_ERROR",umath::to_integral(::sql::Result::ErBackupLogWriteError)},
		{"RESULT_ER_TABLESPACE_NOT_EMPTY",umath::to_integral(::sql::Result::ErTablespaceNotEmpty)},
		{"RESULT_ER_BACKUP_CAT_ENUM",umath::to_integral(::sql::Result::ErBackupCatEnum)},
		{"RESULT_ER_BACKUP_CANT_RESTORE_TS",umath::to_integral(::sql::Result::ErBackupCantRestoreTs)},
		{"RESULT_ER_BACKUP_TS_CHANGE",umath::to_integral(::sql::Result::ErBackupTsChange)},
		{"RESULT_ER_BACKUP_GET_META_TS",umath::to_integral(::sql::Result::ErBackupGetMetaTs)},
		{"RESULT_ER_TABLESPACE_DATAFILE_EXIST",umath::to_integral(::sql::Result::ErTablespaceDatafileExist)},
		{"RESULT_ER_BACKUP_CATALOG_ADD_TS",umath::to_integral(::sql::Result::ErBackupCatalogAddTs)},
		{"RESULT_ER_DEBUG_SYNC_TIMEOUT",umath::to_integral(::sql::Result::ErDebugSyncTimeout)},
		{"RESULT_ER_DEBUG_SYNC_HIT_LIMIT",umath::to_integral(::sql::Result::ErDebugSyncHitLimit)},
		{"RESULT_ER_BACKUP_FAILED_TO_INIT_COMPRESSION",umath::to_integral(::sql::Result::ErBackupFailedToInitCompression)},
		{"RESULT_ER_BACKUP_OBTAIN_NAME_LOCK_FAILED",umath::to_integral(::sql::Result::ErBackupObtainNameLockFailed)},
		{"RESULT_ER_BACKUP_RELEASE_NAME_LOCK_FAILED",umath::to_integral(::sql::Result::ErBackupReleaseNameLockFailed)},
		{"RESULT_ER_BACKUP_BACKUPDIR",umath::to_integral(::sql::Result::ErBackupBackupdir)}
	});

	auto &modSqlite = l.RegisterLibrary("sql");
	auto classDefConnection = luabind::class_<std::shared_ptr<::sql::BaseConnection>>("Connection");
	classDefConnection.def("Close",&Connection::Close);
	classDefConnection.def("Execute",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&,const luabind::object&)>(&Connection::Execute));
	classDefConnection.def("Execute",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&)>(&Connection::Execute));
	classDefConnection.def("Execute",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&)>(&Connection::Execute));
	classDefConnection.def("ExecuteQueryAndFetchRows",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&,const luabind::object&)>(&Connection::ExecuteQueryAndFetchRows));
	classDefConnection.def("ExecuteQueryAndFetchRows",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&)>(&Connection::ExecuteQueryAndFetchRows));
	classDefConnection.def("ExecuteQueryAndFetchRows",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&)>(&Connection::ExecuteQueryAndFetchRows));
	classDefConnection.def("GetLastInsertRowId",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const luabind::object&)>(&Connection::GetLastInsertRowId));
	classDefConnection.def("GetLastInsertRowId",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&)>(&Connection::GetLastInsertRowId));
	classDefConnection.def("CreateStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&)>(&Connection::CreateStatement));
	classDefConnection.def("CreateStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&)>(&Connection::CreateStatement));
	classDefConnection.def("CreatePreparedStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&)>(&Connection::CreatePreparedStatement));
	classDefConnection.def("CreatePreparedStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseConnection>&,const std::string&,const luabind::object&)>(&Connection::CreatePreparedStatement));
	classDefConnection.def("Wait",&Connection::Wait);
	classDefConnection.def("CallOnQueueComplete",&Connection::CallOnQueueComplete);
	classDefConnection.def("GetResultCode",&Connection::GetResultCode);
	classDefConnection.def("GetResultErrorMessage",&Connection::GetResultErrorMessage);

	/*classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::PrepareStatement));
	classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::PrepareStatement));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::Exec));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::Exec));
	classDefConnection.def("GetResult",&Connection::GetResult);
	classDefConnection.def("GetResultMessage",&Connection::GetResultMessage);
	classDefConnection.def("IsDatabaseReadOnly",&Connection::IsDatabaseReadOnly);
	classDefConnection.def("DatabaseExists",&Connection::DatabaseExists);
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("DeleteFunction",&Connection::DeleteFunction);*/
	modSqlite[classDefConnection];

	auto classDefStatement = luabind::class_<std::shared_ptr<::sql::BaseStatement>>("Statement");
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::Step));
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::Step));
	classDefStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::ExecuteQuery));
	classDefStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::ExecuteQuery));
	classDefStatement.def("Finalize",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::Finalize));
	classDefStatement.def("Finalize",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::Finalize));
	classDefStatement.def("Reset",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::Reset));
	classDefStatement.def("Reset",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::Reset));
	classDefStatement.def("FindColumn",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const std::string&,const luabind::object&)>(&Statement::FindColumn));
	classDefStatement.def("FindColumn",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const std::string&)>(&Statement::FindColumn));
	classDefStatement.def("IsReadOnly",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::IsReadOnly));
	classDefStatement.def("IsReadOnly",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::IsReadOnly));
	classDefStatement.def("GetType",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetType));
	classDefStatement.def("GetType",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetType));
	classDefStatement.def("GetDouble",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetDouble));
	classDefStatement.def("GetDouble",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetDouble));
	classDefStatement.def("GetInt",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetInt));
	classDefStatement.def("GetInt",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetInt));
	classDefStatement.def("GetInt64",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetInt64));
	classDefStatement.def("GetInt64",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetInt64));
	classDefStatement.def("GetText",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetText));
	classDefStatement.def("GetText",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetText));
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetBlob));
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetBlob));
	classDefStatement.def("GetColumnCount",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::GetColumnCount));
	classDefStatement.def("GetColumnCount",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::GetColumnCount));
	classDefStatement.def("GetColumnName",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetColumnName));
	classDefStatement.def("GetColumnName",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetColumnName));
	classDefStatement.def("GetRowCount",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const luabind::object&)>(&Statement::GetRowCount));
	classDefStatement.def("GetRowCount",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&)>(&Statement::GetRowCount));
	classDefStatement.def("GetValue",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t,const luabind::object&)>(&Statement::GetValue));
	classDefStatement.def("GetValue",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,int32_t)>(&Statement::GetValue));
	classDefStatement.def("ExecuteQueryAndFetchRows",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const std::string&,const luabind::object&)>(&Statement::ExecuteQueryAndFetchRows));
	classDefStatement.def("ExecuteQueryAndFetchRows",static_cast<void(*)(lua_State*,std::shared_ptr<::sql::BaseStatement>&,const std::string&)>(&Statement::ExecuteQueryAndFetchRows));
	modSqlite[classDefStatement];

	auto classDefBlob = luabind::class_<std::shared_ptr<::sqlite::Blob>>("Blob");
	classDefBlob.def("Close",&Blob::Close);
	classDefBlob.def("GetBytes",&Blob::GetBytes);
	classDefBlob.def("Read",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,int32_t,int32_t,const luabind::object&)>(&Blob::Read));
	classDefBlob.def("Read",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,int32_t,int32_t)>(&Blob::Read));
	classDefBlob.def("Write",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,::DataStream&,int32_t,int32_t,const luabind::object&)>(&Blob::Write));
	classDefBlob.def("Write",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,::DataStream&,int32_t,int32_t)>(&Blob::Write));
	classDefBlob.def("Reopen",&Blob::Reopen);
	modSqlite[classDefBlob];

	auto classDefValue = luabind::class_<::sql::PValue>("Value");
	classDefValue.def("GetType",&Value::GetType);
	classDefValue.def("GetDouble",&Value::GetDouble);
	classDefValue.def("GetInt",&Value::GetInt);
	classDefValue.def("GetInt64",&Value::GetInt64);
	classDefValue.def("GetText",&Value::GetText);
	classDefValue.def("GetBlob",&Value::GetBlob);
	classDefValue.def("ToString",&Value::ToString);
	modSqlite[classDefValue];
	/*
	auto &modSqlite = l.RegisterLibrary("sqlite");
	auto classDefConnection = luabind::class_<std::shared_ptr<::sqlite::Connection>>("Connection");
	classDefConnection.def("Close",&Connection::Close);
	classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::PrepareStatement));
	classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::PrepareStatement));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::Exec));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::Exec));
	classDefConnection.def("GetResult",&Connection::GetResult);
	classDefConnection.def("GetResultMessage",&Connection::GetResultMessage);
	classDefConnection.def("IsDatabaseReadOnly",&Connection::IsDatabaseReadOnly);
	classDefConnection.def("DatabaseExists",&Connection::DatabaseExists);
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("DeleteFunction",&Connection::DeleteFunction);
	modSqlite[classDefConnection];

	auto classDefStatement = luabind::class_<std::shared_ptr<::sqlite::Statement>>("Statement");
	classDefStatement.def("Finalize",&Statement::Finalize);
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,const luabind::object&)>(&Statement::Step));
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&)>(&Statement::Step));
	classDefStatement.def("Reset",&Statement::Reset);
	classDefStatement.def("GetType",&Statement::GetType);
	classDefStatement.def("GetBytes",&Statement::GetBytes);
	classDefStatement.def("GetDouble",&Statement::GetDouble);
	classDefStatement.def("GetInt",&Statement::GetInt);
	classDefStatement.def("GetInt64",&Statement::GetInt64);
	classDefStatement.def("GetText",&Statement::GetText);
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,int32_t,const luabind::object&)>(&Statement::GetBlob));
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,int32_t)>(&Statement::GetBlob));
	classDefStatement.def("GetCount",&Statement::GetCount);
	classDefStatement.def("GetColumnName",&Statement::GetColumnName);

	classDefStatement.def("BindDouble",&Statement::BindDouble);
	classDefStatement.def("BindInt",&Statement::BindInt);
	classDefStatement.def("BindInt64",&Statement::BindInt64);
	classDefStatement.def("BindNull",&Statement::BindNull);
	classDefStatement.def("BindText",&Statement::BindText);
	classDefStatement.def("BindBlob",&Statement::BindBlob);
	classDefStatement.def("ClearBindings",&Statement::ClearBindings);
	classDefStatement.def("IsReadOnly",&Statement::IsReadOnly);
	classDefStatement.def("GetSQLText",&Statement::GetSQLText);
	classDefStatement.def("GetExpandedSQLText",&Statement::GetExpandedSQLText);
	classDefStatement.def("GetDatabaseName",&Statement::GetDatabaseName);
	classDefStatement.def("GetTableName",&Statement::GetTableName);
	classDefStatement.def("GetOriginName",&Statement::GetOriginName);
	modSqlite[classDefStatement];
	*/
}

// Also used in engine in lfile.cpp
static bool add_addon_prefix(lua_State *l,std::string &path)
{
	auto fname = FileManager::GetCanonicalizedPath(Lua::get_current_file(l));
	if(fname.length() < 8 || ustring::compare(fname.c_str(),"addons\\",false,7) == false)
	{
		std::cout<<"WARNING: SQLite databases can only be opened by Lua-scripts inside an addon!"<<std::endl;
		return false;
	}
	auto br = fname.find(FileManager::GetDirectorySeparator(),8);
	auto prefix = ustring::sub(fname,0,br +1);
	path = FileManager::GetProgramPath() +'\\' +prefix +path;
	return true;
}

static void initialize_connection(lua_State *l,const std::shared_ptr<::sql::BaseConnection> &con)
{
	CallbackHandle hCb = FunctionCallback<void>::Create(nullptr);
	std::weak_ptr<::sql::BaseConnection> wpCon = con;
	hCb.get<Callback<void>>()->SetFunction([hCb,wpCon]() mutable {
		if(wpCon.expired() == false)
			wpCon.lock()->Poll();
		else if(hCb.IsValid())
			hCb.Remove();
	});
	ishared::add_callback(l,ishared::Callback::Think,hCb);
}

int32_t Lua::sql::create_value(lua_State *l)
{
	auto argIdx = 1;
	auto type = static_cast<::sql::Type>(Lua::CheckInt(l,argIdx++));
	auto val = ::sql::PValue{nullptr};
	switch(type)
	{
		case ::sql::Type::Double:
		{
			auto v = Lua::CheckNumber(l,argIdx++);
			val = std::make_shared<::sql::Value>(static_cast<long double>(v));
			break;
		}
		case ::sql::Type::Integer:
		{
			auto v = Lua::CheckInt(l,argIdx++);
			val = std::make_shared<::sql::Value>(static_cast<int32_t>(v));
			break;
		}
		case ::sql::Type::BigInt:
		{
			auto v = Lua::CheckInt(l,argIdx++);
			val = std::make_shared<::sql::Value>(static_cast<int64_t>(v));
			break;
		}
		case ::sql::Type::VarChar:
		{
			std::string v = Lua::CheckString(l,argIdx++);
			val = std::make_shared<::sql::Value>(v);
			break;
		}
		case ::sql::Type::Binary:
		{
			auto *v = Lua::CheckDataStream(l,argIdx++);
			val = std::make_shared<::sql::Value>(*v);
			break;
		}
		case ::sql::Type::Null:
		{
			val = std::make_shared<::sql::Value>(nullptr);
			break;
		}
		default:
		{
			std::cout<<"WARNING: Invalid type '"<<umath::to_integral(type)<<" used for SQL value!"<<std::endl;
			return 0;
		}
	}
	Lua::Push<::sql::PValue>(l,val);
	return 1;
}

int32_t Lua::sql::open(lua_State *l)
{
	int32_t argIdx = 1;
	auto conType = static_cast<::sql::ConnectionType>(Lua::CheckInt(l,argIdx++));
	switch(conType)
	{
		case ::sql::ConnectionType::MySQL:
		{
			std::string ip = Lua::CheckString(l,argIdx++);
			std::string user = Lua::CheckString(l,argIdx++);
			std::string password = Lua::CheckString(l,argIdx++);
			std::string dbName = Lua::CheckString(l,argIdx++);
			auto con = std::shared_ptr<::sql::BaseConnection>(new ::mysql::Connection());
			if(Lua::IsSet(l,argIdx))
			{
				Lua::CheckFunction(l,argIdx);
				auto oCallback = luabind::object(luabind::from_stack(l,argIdx++));
				initialize_connection(l,con);
				static_cast<::mysql::Connection&>(*con).Connect(ip,user,password,dbName,[oCallback,l,con](::sql::Result result) {
					oCallback.push(l);
					Lua::PushInt(l,umath::to_integral(result));
					auto numArgs = 1;
					if(result == ::sql::Result::Ok)
					{
						Lua::Push<std::shared_ptr<::sql::BaseConnection>>(l,con);
						++numArgs;
					}
					if(ishared::protected_lua_call(l,numArgs,0) == true)
						;
				});
				return 0;
			}
			auto result = static_cast<::mysql::Connection&>(*con).Connect(ip,user,password,dbName);
			Lua::PushInt(l,umath::to_integral(result));
			auto numRet = 1;
			if(result == ::sql::Result::Ok)
			{
				Lua::Push<std::shared_ptr<::sql::BaseConnection>>(l,con);
				initialize_connection(l,con);
				++numRet;
			}
			else
			{
				auto &msg = con->GetResultErrorMessage();
				Lua::PushString(l,msg);
				++numRet;
			}
			return numRet;
		}
		case ::sql::ConnectionType::SQLite:
		{
			std::string fname = Lua::CheckString(l,argIdx++);
			if(fname != ":memory:")
			{
				if(add_addon_prefix(l,fname) == false)
					return 0;
			}
			auto con = std::make_shared<sqlite::Connection>();
			auto flags = std::unique_ptr<int32_t>(nullptr);
			auto fCallback = std::shared_ptr<luabind::object>(nullptr);
			if(Lua::IsSet(l,argIdx))
			{
				if(Lua::IsFunction(l,argIdx))
					fCallback = std::make_shared<luabind::object>(luabind::object(luabind::from_stack(l,argIdx++)));
				else
				{
					flags = std::make_unique<int32_t>(Lua::CheckInt(l,argIdx++));
					if(Lua::IsFunction(l,argIdx))
						fCallback = std::make_shared<luabind::object>(luabind::object(luabind::from_stack(l,argIdx++)));
				}
			}
			if(fCallback != nullptr)
			{
				initialize_connection(l,con);
				auto &oCallback = *fCallback;
				static_cast<::sqlite::Connection&>(*con).Connect(fname,[oCallback,l,con](::sql::Result result) {
					oCallback.push(l);
					Lua::PushInt(l,umath::to_integral(result));
					auto numArgs = 1;
					if(result == ::sql::Result::Ok)
					{
						Lua::Push<std::shared_ptr<::sql::BaseConnection>>(l,con);
						++numArgs;
					}
					if(ishared::protected_lua_call(l,numArgs,0) == true)
						;
				},flags.get());
				return 0;
			}
			else
			{
				auto result = con->Connect(fname,flags.get());
				Lua::PushInt(l,umath::to_integral(result));

				if(result == ::sql::Result::Ok)
				{
					Lua::Push<std::shared_ptr<::sql::BaseConnection>>(l,con);
					initialize_connection(l,con);
					return 2;
				}
				else
				{
					auto &msg = con->GetResultErrorMessage();
					Lua::PushString(l,msg);
					return 2;
				}
				return 1;
			}
		}
	}
	return 0;
}

