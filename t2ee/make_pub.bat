
rmdir /S /Q \pub\logsystem

mkdir \pub\logsystem\aggregator
mkdir \pub\logsystem\collector
mkdir \pub\logsystem\sql
mkdir \pub\logsystem\map
mkdir \pub\logsystem\dictexplorer
mkdir \pub\logsystem\import

copy /Y CHANGES                      \pub\logsystem\CHANGES
copy /Y README                       \pub\logsystem\README

:: Build
set MSDEV=D:\PROGRA~1\MICROS~1\Common7\IDE\devenv.com
%MSDEV% ../project/logsystem\t2ee\logsystem\logsystem.sln /Build Release
%MSDEV% ../project/collector\collector\collector.sln /Build Release
%MSDEV% ../project/import\t2ee\import\import.sln /Build Release

:: aggregator
copy /Y ..\project\logsystem\t2ee\logsystem\Release\logsystem.dll   \pub\logsystem\aggregator\logsystem.dll
copy /Y ..\project\logsystem\t2ee\logsystem\Release\aggregator.dll  \pub\logsystem\aggregator\aggregator.dll
copy /Y ..\project\logsystem\t2ee\logsystem\Release\parsertc50.dll  \pub\logsystem\aggregator\parsertc50.dll
copy /Y ..\project\logsystem\t2ee\logsystem\Release\settingdb.dll   \pub\logsystem\aggregator\settingdb.dll
copy /Y ..\aggregator\public\include\aggregator_pub.xml             \pub\logsystem\aggregator\aggregator.xml

:: sql
copy /Y ..\aggregator\public\include\tls.sql                      \pub\logsystem\sql\tls.sql
copy /Y ..\aggregator\public\include\tls_his.sql                  \pub\logsystem\sql\tls_his.sql

:: mysql
copy /Y e:\dll\libmysql.dll.release                               \pub\logsystem\import\libmysql.dll

:: collector
copy /Y ..\project\collector\collector\Release\collector.exe        \pub\logsystem\collector\collector.exe

:: taapi engine
copy /Y \pub\taapi\taapi.dll              \pub\logsystem\collector\taapi.dll
copy /Y ..\collector\public\include\taapi.xml              \pub\logsystem\collector\taapi.xml

:: dcit explorer
:: D:\PROGRA~1\MICROS~3\VC98\BIN\VCVARS32.BAT
:: Build
set MSDEV=D:\PROGRA~1\MICROS~3\Common\msdev98\bin\MSDEV.EXE
%MSDEV% ExportAQDict\ExportAQDict.dsw /MAKE "ExportAQDict - Win32 Release"

copy /Y ExportAQDict\Release\ExportAQDict.exe   \pub\logsystem\dictexplorer\dictexplorer.exe

:: import
copy /Y ..\project\import\t2ee\import\Release\import.exe  \pub\logsystem\import\import.exe
copy /Y e:\DevKits\T2EE_SDK\wtcommlib\lib\WTCommLib.dll   \pub\logsystem\import\wtcommlib.dll

:: map
copy /Y ..\project\logsystem\t2ee\logsystem\Release\logsystem.map   \pub\logsystem\map\logsystem.map
copy /Y ..\project\logsystem\t2ee\logsystem\Release\aggregator.map  \pub\logsystem\map\aggregator.map
copy /Y ..\project\logsystem\t2ee\logsystem\Release\parsertc50.map  \pub\logsystem\map\parsertc50.map
copy /Y ..\project\logsystem\t2ee\logsystem\Release\settingdb.map   \pub\logsystem\map\settingdb.map
copy /Y ..\project\collector\collector\Release\collector.map        \pub\logsystem\map\collector.map
copy /Y ExportAQDict\Release\ExportAQDict.map                       \pub\logsystem\map\dictexplorer.map
copy /Y ..\project\import\t2ee\import\Release\import.map            \pub\logsystem\map\import.map

copy /Y ..\project\logsystem\t2ee\logsystem\Release\logsystem.dll.pdb   \pub\logsystem\map\logsystem.dll.pdb
copy /Y ..\project\logsystem\t2ee\logsystem\Release\aggregator.dll.pdb  \pub\logsystem\map\aggregator.dll.pdb
copy /Y ..\project\logsystem\t2ee\logsystem\Release\parsertc50.dll.pdb  \pub\logsystem\map\parsertc50.dll.pdb
copy /Y ..\project\logsystem\t2ee\logsystem\Release\settingdb.dll.pdb   \pub\logsystem\map\settingdb.dll.pdb
copy /Y ..\project\collector\collector\Release\collector.exe.pdb        \pub\logsystem\map\collector.exe.pdb
copy /Y ExportAQDict\Release\ExportAQDict.exe.pdb                       \pub\logsystem\map\dictexplorer.exe.pdb
copy /Y ..\project\import\t2ee\import\Release\import.exe.pdb            \pub\logsystem\map\import.exe.pdb
