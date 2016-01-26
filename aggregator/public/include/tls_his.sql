-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server version:               5.1.40 - Source distribution
-- Server OS:                    Win32
-- HeidiSQL version:             7.0.0.4206
-- Date/time:                    2013-10-28 15:20:09
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping database structure for tls_his
CREATE DATABASE IF NOT EXISTS `tls_his` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `tls_his`;


-- Dumping structure for table tls_his.t_tc50_logon_ans_13
CREATE TABLE IF NOT EXISTS `t_tc50_logon_ans_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `timeA` int(10) NOT NULL,
  `timeB` int(10) NOT NULL,
  `queue` int(10) NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_logon_req_13
CREATE TABLE IF NOT EXISTS `t_tc50_logon_req_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `XT_GTLB` char(2) NOT NULL,
  `KHH` varchar(32) NOT NULL,
  `KHMC` varchar(32) NOT NULL,
  `ZHLB` int(10) NOT NULL,
  `ZJZH` varchar(16) NOT NULL,
  `SHGD` varchar(16) NOT NULL,
  `SZGD` varchar(16) NOT NULL,
  `XT_CLITYPE` int(10) NOT NULL,
  `XT_CLIVER` varchar(16) NOT NULL,
  `XT_VIPFLAG` int(10) NOT NULL,
  `XT_MACHINEINFO` varchar(1024) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_log_failed_13
CREATE TABLE IF NOT EXISTS `t_tc50_log_failed_13` (
  `parser` varchar(16) NOT NULL,
  `collector` varchar(32) NOT NULL,
  `file_id` int(10) NOT NULL,
  `file_pos_start` bigint(20) NOT NULL,
  `log_len` int(10) NOT NULL,
  `timestamp` int(10) NOT NULL,
  `log_date` smallint(5) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_log_info_13
CREATE TABLE IF NOT EXISTS `t_tc50_log_info_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL COMMENT '解析者',
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `info` varchar(64) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `op_organization` varchar(64) NOT NULL,
  `op_account` varchar(32) NOT NULL,
  `reason` varchar(64) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_sc_ans_13
CREATE TABLE IF NOT EXISTS `t_tc50_sc_ans_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `timeA` int(10) NOT NULL,
  `timeB` int(10) NOT NULL,
  `queue` int(10) NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_sc_req_13
CREATE TABLE IF NOT EXISTS `t_tc50_sc_req_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `XT_GTLB` char(2) NOT NULL,
  `CA_KHH` varchar(64) NOT NULL,
  `CA_KHMC` varchar(64) NOT NULL,
  `CA_VER` varchar(64) NOT NULL,
  `CA_AQJB` varchar(64) NOT NULL,
  `CA_TXMM` varchar(64) NOT NULL,
  `CA_ISVIPHOST` varchar(64) NOT NULL,
  `CA_JQTZM` varchar(64) NOT NULL,
  `CA_SLOTSN` varchar(64) NOT NULL,
  `CA_CID` varchar(64) NOT NULL,
  `CA_CERTREQ` varchar(64) NOT NULL,
  `CA_USERCERDN` varchar(64) NOT NULL,
  `CA_ZSQSRQ` varchar(64) NOT NULL,
  `CA_ZSJZRQ` varchar(64) NOT NULL,
  `CA_CERTSN` varchar(64) NOT NULL,
  `CA_CERTINFO` varchar(64) NOT NULL,
  `CA_MACHINENAME` varchar(64) NOT NULL,
  `CA_DLSJ` varchar(64) NOT NULL,
  `CA_LASTIP` varchar(64) NOT NULL,
  `CA_MAC` varchar(64) NOT NULL,
  `CA_CSCS` varchar(64) NOT NULL,
  `CA_RESV` varchar(64) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_trade_ans_13
CREATE TABLE IF NOT EXISTS `t_tc50_trade_ans_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `timeA` int(10) NOT NULL,
  `timeB` int(10) NOT NULL,
  `queue` int(10) NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `WTBH` varchar(32) NOT NULL,
  `XT_CHECKRISKFLAG` int(11) NOT NULL,
  `RETINFO` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table tls_his.t_tc50_trade_req_13
CREATE TABLE IF NOT EXISTS `t_tc50_trade_req_13` (
  `parser` varchar(16) NOT NULL COMMENT 'lookup',
  `collector` varchar(32) NOT NULL COMMENT 'lookup',
  `file_id` int(10) NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',
  `log_len` int(10) NOT NULL COMMENT '文件位置',
  `timestamp` int(10) NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) NOT NULL,
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',
  `log_time` int(10) NOT NULL,
  `ip` bigint(20) NOT NULL,
  `mac` bigint(20) NOT NULL,
  `thread_id` smallint(5) NOT NULL,
  `channel_id` int(10) NOT NULL,
  `trans_id` int(10) NOT NULL,
  `req_type` tinyint(3) NOT NULL,
  `func_id` smallint(5) NOT NULL,
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',
  `KHH` varchar(32) NOT NULL,
  `KHMC` varchar(32) NOT NULL,
  `ZHLB` int(10) NOT NULL,
  `ZJZH` varchar(16) NOT NULL,
  `GDDM` varchar(16) NOT NULL,
  `OP_WTFS` int(10) NOT NULL,
  `WTBH` varchar(32) NOT NULL,
  `WTFS` int(10) NOT NULL,
  `ZQDM` varchar(16) NOT NULL,
  `MMBZ` int(10) NOT NULL,
  `JYDW` int(10) NOT NULL,
  `WTSL` int(10) NOT NULL,
  `WTJG` decimal(10,3) NOT NULL,
  `WTRQ` int(10) NOT NULL,
  `WTSJ` int(10) NOT NULL,
  `XT_CHECKRISKFLAG` int(10) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL
) ENGINE=BDE DEFAULT CHARSET=utf8;

-- Data exporting was unselected.
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
