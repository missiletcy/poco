//
// ODBCSQLiteTest.cpp
//
// $Id: //poco/1.3/Data/ODBC/testsuite/src/ODBCSQLiteTest.cpp#4 $
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "ODBCSQLiteTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/Exception.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/BLOB.h"
#include "Poco/Data/StatementImpl.h"
#include "Poco/Data/ODBC/Connector.h"
#include "Poco/Data/ODBC/Utility.h"
#include "Poco/Data/ODBC/Diagnostics.h"
#include "Poco/Data/ODBC/ODBCException.h"
#include "Poco/Data/ODBC/ODBCStatementImpl.h"
#include <sqltypes.h>
#include <iostream>


using namespace Poco::Data;
using Poco::Data::ODBC::Utility;
using Poco::Data::ODBC::ConnectionException;
using Poco::Data::ODBC::StatementException;
using Poco::Data::ODBC::StatementDiagnostics;
using Poco::format;
using Poco::NotFoundException;


const bool ODBCSQLiteTest::bindValues[8] = {true, true, true, false, false, true, false, false};
Poco::SharedPtr<Poco::Data::Session> ODBCSQLiteTest::_pSession = 0;
Poco::SharedPtr<SQLExecutor> ODBCSQLiteTest::_pExecutor = 0;
std::string ODBCSQLiteTest::_dbConnString;
Poco::Data::ODBC::Utility::DriverMap ODBCSQLiteTest::_drivers;


ODBCSQLiteTest::ODBCSQLiteTest(const std::string& name): 
	CppUnit::TestCase(name)
{
	static bool beenHere = false;

	ODBC::Connector::registerConnector();
	if (_drivers.empty()) 
	{
		Utility::drivers(_drivers);
		checkODBCSetup();
	}
	if (!_pSession && !_dbConnString.empty() && !beenHere)
	{
		try
		{
			_pSession = new Session(SessionFactory::instance().create(ODBC::Connector::KEY, _dbConnString));
			if (_pSession && _pSession->isConnected()) 
			std::cout << "*** Connected to " << _dbConnString << std::endl;
			if (!_pExecutor) _pExecutor = new SQLExecutor("SQLite SQL Executor", _pSession);
		}catch (ConnectionException& ex)
		{
			std::cout << "!!! WARNING: Connection failed. SQLite tests will fail !!!" << std::endl;
			std::cout << ex.toString() << std::endl;
		}
	}
	else 
	if (!_pSession && !beenHere) 
		std::cout << "!!! WARNING: No driver or DSN found. SQLite tests will fail !!!" << std::endl;

	beenHere = true;
}


ODBCSQLiteTest::~ODBCSQLiteTest()
{
	ODBC::Connector::unregisterConnector();
}


void ODBCSQLiteTest::testBareboneODBC()
{
	if (!_pSession) fail ("Test not available.");

	std::string tableCreateString = "CREATE TABLE Test "
		"(First VARCHAR(30),"
		"Second VARCHAR(30),"
		"Third BLOB,"
		"Fourth INTEGER,"
		"Fifth REAL,"
		"Sixth TIMESTAMP)";

	_pExecutor->bareboneODBCTest(_dbConnString, tableCreateString, SQLExecutor::PB_IMMEDIATE, SQLExecutor::DE_MANUAL);
	_pExecutor->bareboneODBCTest(_dbConnString, tableCreateString, SQLExecutor::PB_IMMEDIATE, SQLExecutor::DE_BOUND);
	_pExecutor->bareboneODBCTest(_dbConnString, tableCreateString, SQLExecutor::PB_AT_EXEC, SQLExecutor::DE_MANUAL);
	_pExecutor->bareboneODBCTest(_dbConnString, tableCreateString, SQLExecutor::PB_AT_EXEC, SQLExecutor::DE_BOUND);
}


void ODBCSQLiteTest::testSimpleAccess()
{
	if (!_pSession) fail ("Test not available.");

	std::string tableName("Person");
	std::string result;

	_pSession->setFeature("autoBind", false);

	recreatePersonTable();

	try { *_pSession << "SELECT name FROM sqlite_master WHERE tbl_name=?", into(result), use(tableName), now; }
	catch(StatementException& ex) {	std::cout << ex.toString() << std::endl; }

	assert (result == tableName);

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->simpleAccess();
		i += 2;
	}
}


void ODBCSQLiteTest::testComplexType()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->complexType();
		i += 2;
	}
}


void ODBCSQLiteTest::testSimpleAccessVector()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->simpleAccessVector();
		i += 2;
	}
}


void ODBCSQLiteTest::testComplexTypeVector()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->complexTypeVector();
		i += 2;
	}	
}


void ODBCSQLiteTest::testInsertVector()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateStringsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->insertVector();
		i += 2;
	}	
}


void ODBCSQLiteTest::testInsertEmptyVector()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateStringsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->insertEmptyVector();
		i += 2;
	}	
}


void ODBCSQLiteTest::testInsertSingleBulk()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->insertSingleBulk();
		i += 2;
	}	
}


void ODBCSQLiteTest::testInsertSingleBulkVec()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->insertSingleBulkVec();
		i += 2;
	}	
}


void ODBCSQLiteTest::testLimit()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->limits();
		i += 2;
	}
}


void ODBCSQLiteTest::testLimitZero()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->limitZero();
		i += 2;
	}	
}


void ODBCSQLiteTest::testLimitOnce()
{
	if (!_pSession) fail ("Test not available.");

	recreateIntsTable();
	_pExecutor->limitOnce();
	
}


void ODBCSQLiteTest::testLimitPrepare()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->limitPrepare();
		i += 2;
	}
}



void ODBCSQLiteTest::testPrepare()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateIntsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->prepare();
		i += 2;
	}
}


void ODBCSQLiteTest::testSetSimple()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->setSimple();
		i += 2;
	}
}


void ODBCSQLiteTest::testSetComplex()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->setComplex();
		i += 2;
	}
}


void ODBCSQLiteTest::testSetComplexUnique()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->setComplexUnique();
		i += 2;
	}
}

void ODBCSQLiteTest::testMultiSetSimple()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->multiSetSimple();
		i += 2;
	}
}


void ODBCSQLiteTest::testMultiSetComplex()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->multiSetComplex();
		i += 2;
	}	
}


void ODBCSQLiteTest::testMapComplex()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->mapComplex();
		i += 2;
	}
}


void ODBCSQLiteTest::testMapComplexUnique()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->mapComplexUnique();
		i += 2;
	}
}


void ODBCSQLiteTest::testMultiMapComplex()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->multiMapComplex();
		i += 2;
	}
}


void ODBCSQLiteTest::testSelectIntoSingle()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->selectIntoSingle();
		i += 2;
	}
}


void ODBCSQLiteTest::testSelectIntoSingleStep()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->selectIntoSingleStep();
		i += 2;
	}	
}


void ODBCSQLiteTest::testSelectIntoSingleFail()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->selectIntoSingleFail();
		i += 2;
	}	
}


void ODBCSQLiteTest::testLowerLimitOk()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->lowerLimitOk();
		i += 2;
	}	
}


void ODBCSQLiteTest::testSingleSelect()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->singleSelect();
		i += 2;
	}	
}


void ODBCSQLiteTest::testLowerLimitFail()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->lowerLimitFail();
		i += 2;
	}
}


void ODBCSQLiteTest::testCombinedLimits()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->combinedLimits();
		i += 2;
	}
}



void ODBCSQLiteTest::testRange()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->ranges();
		i += 2;
	}
}


void ODBCSQLiteTest::testCombinedIllegalLimits()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->combinedIllegalLimits();
		i += 2;
	}
}



void ODBCSQLiteTest::testIllegalRange()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->illegalRange();
		i += 2;
	}
}


void ODBCSQLiteTest::testEmptyDB()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->emptyDB();
		i += 2;
	}
}


void ODBCSQLiteTest::testBLOB()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonBLOBTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->blob();
		i += 2;
	}
}


void ODBCSQLiteTest::testBLOBStmt()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreatePersonBLOBTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->blobStmt();
		i += 2;
	}
}


void ODBCSQLiteTest::testBool()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateBoolsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->bools();
		i += 2;
	}
}


void ODBCSQLiteTest::testFloat()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateFloatsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->floats();
		i += 2;
	}
}


void ODBCSQLiteTest::testDouble()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateFloatsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->doubles();
		i += 2;
	}
}


void ODBCSQLiteTest::testTuple()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateTuplesTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->tuples();
		i += 2;
	}
}


void ODBCSQLiteTest::testTupleVector()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateTuplesTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->tupleVector();
		i += 2;
	}
}


void ODBCSQLiteTest::testInternalExtraction()
{
	if (!_pSession) fail ("Test not available.");

	for (int i = 0; i < 8;)
	{
		recreateVectorsTable();
		_pSession->setFeature("autoBind", bindValues[i]);
		_pSession->setFeature("autoExtract", bindValues[i+1]);
		_pExecutor->internalExtraction();
		i += 2;
	}
}


void ODBCSQLiteTest::dropTable(const std::string& tableName)
{
	try
	{
		*_pSession << format("DROP TABLE %s", tableName), now;
	}
	catch (StatementException& ex)
	{
		bool ignoreError = false;
		const StatementDiagnostics::FieldVec& flds = ex.diagnostics().fields();
		StatementDiagnostics::Iterator it = flds.begin();
		for (; it != flds.end(); ++it)
		{
			if (1 == it->_nativeError)//(no such table)
			{
				ignoreError = true;
				break;
			}
		}

		if (!ignoreError) 
		{
			std::cout << ex.displayText() << std::endl;
			throw;
		}
	}
}


void ODBCSQLiteTest::recreatePersonTable()
{
	dropTable("Person");
	try { *_pSession << "CREATE TABLE Person (LastName VARCHAR2(30), FirstName VARCHAR2(30), Address VARCHAR2(30), Age INTEGER)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreatePersonTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreatePersonTable()"); }
}


void ODBCSQLiteTest::recreatePersonBLOBTable()
{
	dropTable("Person");
	try { *_pSession << "CREATE TABLE Person (LastName VARCHAR(30), FirstName VARCHAR(30), Address VARCHAR(30), Image BLOB)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreatePersonBLOBTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreatePersonBLOBTable()"); }
}


void ODBCSQLiteTest::recreateIntsTable()
{
	dropTable("Strings");
	try { *_pSession << "CREATE TABLE Strings (str INTEGER)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateIntsTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateIntsTable()"); }
}


void ODBCSQLiteTest::recreateStringsTable()
{
	dropTable("Strings");
	try { *_pSession << "CREATE TABLE Strings (str VARCHAR(30))", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateStringsTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateStringsTable()"); }
}


void ODBCSQLiteTest::recreateBoolsTable()
{
	dropTable("Strings");
	try { *_pSession << "CREATE TABLE Strings (str INTEGER)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateBoolsTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateBoolsTable()"); }
}


void ODBCSQLiteTest::recreateFloatsTable()
{
	dropTable("Strings");
	try { *_pSession << "CREATE TABLE Strings (str REAL)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateFloatsTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateFloatsTable()"); }
}


void ODBCSQLiteTest::recreateTuplesTable()
{
	dropTable("Tuples");
	try { *_pSession << "CREATE TABLE Tuples "
		"(int0 INTEGER, int1 INTEGER, int2 INTEGER, int3 INTEGER, int4 INTEGER, int5 INTEGER, int6 INTEGER, "
		"int7 INTEGER, int8 INTEGER, int9 INTEGER, int10 INTEGER, int11 INTEGER, int12 INTEGER, int13 INTEGER,"
		"int14 INTEGER, int15 INTEGER, int16 INTEGER, int17 INTEGER, int18 INTEGER, int19 INTEGER)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateTuplesTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateTuplesTable()"); }
}


void ODBCSQLiteTest::recreateVectorsTable()
{
	dropTable("Vectors");
	try { *_pSession << "CREATE TABLE Vectors (int0 INTEGER, flt0 REAL, str0 VARCHAR)", now; }
	catch(ConnectionException& ce){ std::cout << ce.toString() << std::endl; fail ("recreateVectorsTable()"); }
	catch(StatementException& se){ std::cout << se.toString() << std::endl; fail ("recreateVectorsTable()"); }
}


void ODBCSQLiteTest::checkODBCSetup()
{
	static bool beenHere = false;

	if (!beenHere)
	{
		beenHere = true;
		bool driverFound = false;
		std::string driverName;

		Utility::DriverMap::iterator itDriver = _drivers.begin();
		for (; itDriver != _drivers.end(); ++itDriver)
		{
			if (((itDriver->first).find("SQLite3") != std::string::npos))
			{
				std::cout << "Driver found: " << itDriver->first 
					<< " (" << itDriver->second << ')' << std::endl;

				driverName = itDriver->first;
				driverFound = true; 
				break;
			}
		}

		if (driverFound) 
		{
			if (_dbConnString.empty() && !driverName.empty())
			{
				_dbConnString = format("Driver=%s;Database=dummy.db;", driverName);
				return;
			}
			else if (driverName.empty())
			{
				std::cout << "SQLite3 driver not found. Tests not available." << std::endl;
				return;
			}
		}
	}
}


void ODBCSQLiteTest::setUp()
{
}


void ODBCSQLiteTest::tearDown()
{
}


CppUnit::Test* ODBCSQLiteTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("ODBCSQLiteTest");

	CppUnit_addTest(pSuite, ODBCSQLiteTest, testBareboneODBC);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSimpleAccess);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testComplexType);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSimpleAccessVector);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testComplexTypeVector);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testInsertVector);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testInsertEmptyVector);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testInsertSingleBulk);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testInsertSingleBulkVec);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLimit);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLimitOnce);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLimitPrepare);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLimitZero);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testPrepare);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSetSimple);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSetComplex);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSetComplexUnique);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testMultiSetSimple);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testMultiSetComplex);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testMapComplex);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testMapComplexUnique);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testMultiMapComplex);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSelectIntoSingle);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSelectIntoSingleStep);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSelectIntoSingleFail);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLowerLimitOk);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testLowerLimitFail);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testCombinedLimits);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testCombinedIllegalLimits);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testRange);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testIllegalRange);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testSingleSelect);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testEmptyDB);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testBLOB);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testBLOBStmt);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testBool);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testFloat);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testDouble);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testTuple);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testTupleVector);
	CppUnit_addTest(pSuite, ODBCSQLiteTest, testInternalExtraction);

	return pSuite;
}

