-- --------------------------------------------------------
-- Host:                         192.168.2.80
-- Server version:               5.1.40 - Source distribution
-- Server OS:                    Win64
-- HeidiSQL version:             7.0.0.4206
-- Date/time:                    2013-10-31 16:24:18
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping database structure for tls
CREATE DATABASE IF NOT EXISTS `tls` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `tls`;


-- Dumping structure for procedure tls.sp_collecotr_set_status
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_collecotr_set_status`(IN `v_collector` VARCHAR(32), IN `v_send_data_len` BIGINT, IN `v_recv_data_len` BIGINT, IN `v_recv_data_len_real` BIGINT, IN `v_logon_time` INT, IN `v_last_keepalive` INT, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
collecotr_set_status_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- SET @logon_time = FROM_UNIXTIME(v_logon_time);
  -- SET @last_keepalive = FROM_UNIXTIME(v_last_keepalive);
  
  INSERT INTO t_collector_status
    (`collector`, `send_data_len`, `recv_data_len`, `recv_data_len_real`, `logon_time`, `last_keepalive`)
  VALUES
    (v_collector, v_send_data_len, v_recv_data_len, v_recv_data_len_real, v_logon_time, v_last_keepalive)
  ON DUPLICATE KEY UPDATE
    `send_data_len` = v_send_data_len
    , `recv_data_len` = v_recv_data_len, `recv_data_len_real` = v_recv_data_len_real
    , `logon_time` = v_logon_time, `last_keepalive` = v_last_keepalive
  ;
  
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Set Collector Status Error.');
    LEAVE collecotr_set_status_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_collector_get_config
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_collector_get_config`(IN `v_collector` VARCHAR(96), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
collector_config_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  SELECT `config_no`, `keepalive`, `rate`, `start`, `end`, `steptime`
  FROM 
  (
    SELECT `keepalive`, `rate`, `start`, `end`, `steptime`, `config_no`
    FROM t_collector_config 
    WHERE collector in (' default', v_collector)
    ORDER BY collector DESC
  ) as t_tcc
  GROUP BY t_tcc.`config_no`;
  
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;
    
    SET err_msg = CONCAT('Collector Config not found.', v_collector);
    LEAVE collector_config_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_collector_get_file
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_collector_get_file`(IN `v_collector` VARCHAR(32), IN `v_name` VARCHAR(64), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
collector_file_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  -- get all file
  IF v_collector IS NULL OR v_collector = '' THEN
    -- 
    IF v_name IS NULL OR v_name = '' THEN
      SELECT `id`, `collector`, `aggregator_path`, `collector_dir`, `collector_name`, `create_time`, `file_size`, `status`, `checksum`
      FROM t_collector_file tcf
      ORDER BY `id` ASC;
    ELSE
      -- get regex name file
      SELECT `id`, `collector`, `collector_dir`, `create_time`
      FROM t_collector_file
      WHERE `collector_name` REGEXP v_name -- like '[0-9a-zA-Z.]*.sto'
      ORDER BY `id` ASC;
    END IF;
  ELSE
    -- get collector file
    SELECT `id`, `file_size`, `status`, `collector_dir`, `collector_name`, `create_time`
    FROM t_collector_file tcf
    WHERE tcf.collector = v_collector
    ORDER BY `id` ASC;
  END IF;
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;
    SET err_msg = CONCAT('Collector File Error.', v_collector);
    LEAVE collector_file_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_collector_get_rule
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_collector_get_rule`(IN `v_collector` VARCHAR(96), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
collector_rule_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  SELECT `dir`, `exclude`, `include`
  FROM 
  (
    SELECT `dir`, `exclude`, `include`, `rule_no` 
    FROM t_collector_rule 
    WHERE collector in (' default', v_collector)
      AND `dir` IS NOT NULL AND `include` is NOT NULL
    ORDER BY collector DESC
  ) as t_tcr
  GROUP BY t_tcr.`rule_no`;
  
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;
    
    SET err_msg = CONCAT('Collector Rule Error.', v_collector);
    LEAVE collector_rule_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_collector_set_fileinfo
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_collector_set_fileinfo`(IN `v_file_id` INT, IN `v_collector` VARCHAR(32), IN `v_dir` VARCHAR(256), IN `v_name` VARCHAR(128), IN `v_ctime` BIGINT, IN `v_path` VARCHAR(512), IN `v_size` BIGINT, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
set_file_info_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  -- begin  
  IF v_collector IS NULL OR v_collector = '' THEN
   -- collector file
    UPDATE t_collector_file tcf
    SET `file_size` = v_size
    WHERE tcf.id = v_file_id;
  ELSE
    INSERT INTO t_collector_file
      (`collector`, `collector_dir`, `collector_name`, `create_time`, `add_time`, `id`, `aggregator_path`)
    VALUES
      (v_collector, v_dir, v_name, v_ctime, CURRENT_TIMESTAMP(), v_file_id, v_path);
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Update File Error.', v_file_id);
    LEAVE set_file_info_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_module_get_setting
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_module_get_setting`(IN `v_module` VARCHAR(64), IN `v_name` VARCHAR(64), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
get_module_setting_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  -- find 
  IF v_name = '' OR v_name IS NULL THEN
    SELECT `name`, `value`, `data`
    FROM t_module_setting
    WHERE `module` = v_module;  
  ELSE
    SELECT `value`, `data`
    FROM t_module_setting
    WHERE `module` = v_module AND `name` = v_name;
  END IF;
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;
    
    SET err_msg = 'get_module_setting failed';
    LEAVE get_module_setting_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_module_set_setting
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_module_set_setting`(IN `v_module` VARCHAR(96), IN `v_name` VARCHAR(96), IN `v_value` BIGINT, IN `v_data` BLOB, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
set_file_info_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  INSERT INTO t_module_setting
    (`module`, `name`, `value`, `data`)
  VALUES
    (v_module, v_name, v_value, v_data)
  ON DUPLICATE KEY UPDATE
    `value` = v_value, `data` = v_data
  ;
  
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Update File Error.', v_file_id);
    LEAVE set_file_info_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_parser_get_logtype_rule
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_parser_get_logtype_rule`(IN `v_type` VARCHAR(64), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
get_log_type_rule_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  -- find 
  SELECT `file_dir`, `file_name`, `judge_rule`, `file_head`
  FROM t_parser_logtype_rule
  WHERE `type` = v_type;
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;
    
    LEAVE get_log_type_rule_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_checksystem
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_checksystem`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_check_system_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions

  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- check table
  -- t_collector_config
  IF NOT EXISTS (
    SELECT  `collector`, `start`, `end`, `config_no`, `steptime`, `keepalive`, `rate` FROM `t_collector_config`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 1329;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_collector_config error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  
  -- t_collector_file
  IF NOT EXISTS (
    SELECT  `id`, `status`, `add_time`, `last_update`, `collector`, `file_size`, `create_time`, `collector_dir`, `collector_name`, `aggregator_path`, `checksum`, LEFT(`block_checksum`, 256) FROM `t_collector_file`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_collector_file error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_collector_rule
  IF NOT EXISTS (
    SELECT  `collector`, `rule_no`, `judge_rule`, LEFT(`dir`, 256), LEFT(`exclude`, 256), LEFT(`include`, 256), LEFT(`comment`, 256) FROM `t_collector_rule`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 1329;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_collector_rule error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  
  -- t_collector_status
  IF NOT EXISTS (
    SELECT  `collector`, `rule_no`, `judge_rule`, LEFT(`dir`, 256), LEFT(`exclude`, 256), LEFT(`include`, 256), LEFT(`comment`, 256) FROM `t_collector_rule`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_collector_status error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_module_setting
  IF NOT EXISTS (
    SELECT  `module`, `name`, `value`, LEFT(`data`, 256), `timestamp` FROM `t_module_setting`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_module_setting error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  
  -- t_parser_logtype_rule
  IF NOT EXISTS (
    SELECT  `type`, `file_dir`, `file_name`, `file_head`, `judge_rule`, `comment`, `encode_type`, `encode_param`, `checksum_type`, `checksum_param` FROM `t_parser_logtype_rule`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 1329;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_parser_logtype_rule error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_sys_log
  IF NOT EXISTS (
    SELECT  `id`, `log_type`, `from`, `extern_data`, `process_status`, `timestamp`, `content` FROM `t_sys_log`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_sys_log error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  
  -- t_sys_resource_status
  IF NOT EXISTS (
    SELECT  `name`, `current_size`, `free_size`, `timestamp`, `last_alive_time`, `last_err_no`, `last_err_msg`, `info` FROM `t_sys_resource_status`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_sys_resource_status error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_sys_rpc_invoke
  IF NOT EXISTS (
    SELECT  `id`, `from`, `target`, `name`, LEFT(`param`, 256), `process_status`, LEFT(`result`, 256), `err_no`, LEFT(`err_msg`, 256), `request`, `reply` FROM `t_sys_rpc_invoke`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_sys_rpc_invoke error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_sys_settings
  IF NOT EXISTS (
    SELECT  `section`, `name`, `value`, LEFT(`comment`, 256) FROM `t_sys_settings`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_sys_settings error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_consts
  IF NOT EXISTS (
    SELECT  `id`, `name`, `descript`, `parentid`, `isvalid`, `log` FROM `t_tc50_consts`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 1329;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_consts error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_logon_ans
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`,  LEFT(`return_msg`, 256),  LEFT(`reserve_a`, 256),  LEFT(`reserve_b`, 256),  LEFT(`reserve_c`, 256),  LEFT(`reserve_d`, 256),  LEFT(`reserve_e`, 256),  LEFT(`reserve_f`, 256) FROM `t_tc50_logon_ans`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_logon_ans error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  
  -- t_tc50_logon_req
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `XT_GTLB`, `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `SHGD`, `SZGD`, `XT_CLITYPE`, `XT_CLIVER`, `XT_VIPFLAG`,  LEFT(`XT_MACHINEINFO`, 256),  LEFT(`reserve_a`, 256),  LEFT(`reserve_b`, 256),  LEFT(`reserve_c`, 256) FROM `t_tc50_logon_req`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_logon_req error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_log_failed
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date` FROM `t_tc50_log_failed`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_log_failed error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_log_info
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `info`, `channel_id`, `ip`, `op_organization`, `op_account`, `reason` FROM `t_tc50_log_info`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_log_info error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_sc_ans
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`,  LEFT(`return_msg`, 256),  LEFT(`reserve_a`, 256),  LEFT(`reserve_b`, 256),  LEFT(`reserve_c`, 256),  LEFT(`reserve_d`, 256),  LEFT(`reserve_e`, 256),  LEFT(`reserve_f`, 256) FROM `t_tc50_sc_ans`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_sc_ans error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_sc_req
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `XT_GTLB`, `CA_KHH`, `CA_KHMC`, `CA_VER`, `CA_AQJB`, `CA_TXMM`, `CA_ISVIPHOST`, `CA_JQTZM`, `CA_SLOTSN`, `CA_CID`, `CA_CERTREQ`, `CA_USERCERDN`, `CA_ZSQSRQ`, `CA_ZSJZRQ`, `CA_CERTSN`, `CA_CERTINFO`, `CA_MACHINENAME`, `CA_DLSJ`, `CA_LASTIP`, `CA_MAC`, `CA_CSCS`, `CA_RESV`,  LEFT(`reserve_a`, 256),  LEFT(`reserve_b`, 256),  LEFT(`reserve_c`, 256) FROM `t_tc50_sc_req`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_sc_req error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_trade_ans
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`,  LEFT(`return_msg`, 256), `WTBH`, `XT_CHECKRISKFLAG`,  LEFT(`RETINFO`, 256),  LEFT(`reserve_d`, 256),  LEFT(`reserve_e`, 256),  LEFT(`reserve_f`, 256) FROM `t_tc50_trade_ans`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_trade_ans error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- t_tc50_trade_req
  IF NOT EXISTS (
    SELECT `id`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `GDDM`, `OP_WTFS`, `WTBH`, `WTFS`, `ZQDM`, `MMBZ`, `JYDW`, `WTSL`, `WTJG`, `WTRQ`, `WTSJ`, `XT_CHECKRISKFLAG`,  LEFT(`reserve_a`, 256),  LEFT(`reserve_b`, 256),  LEFT(`reserve_c`, 256) FROM `t_tc50_trade_req`
    LIMIT 1
  ) THEN
    SET mysql_err_code = 0;
  END IF;
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 't_tc50_trade_req error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;
  -- check 
  SET @db_name = DATABASE();
  SET @sp_count = 24;
  SELECT COUNT(1)
  INTO @sp_count_real
  FROM `mysql`.`proc`
  WHERE `db` = @db_name
    AND `type` = 'PROCEDURE'
    AND `name` IN (
      'sp_collecotr_set_status'
      , 'sp_collector_get_config'
      , 'sp_collector_get_file'
      , 'sp_collector_get_rule'
      , 'sp_collector_set_fileinfo'
      , 'sp_module_get_setting'
      , 'sp_module_set_setting'
      , 'sp_parser_get_logtype_rule'
      , 'sp_sys_checksystem'
      , 'sp_sys_get_rpc'
      , 'sp_sys_get_setting'
      , 'sp_sys_log'
      , 'sp_sys_set_checkpoint'
      , 'sp_sys_set_resource_status'
      , 'sp_sys_set_rpc'
      , 'sp_tc50_get_log_rule'
      , 'sp_tc50_set_failed_log'
      , 'sp_tc50_set_info_log'
      , 'sp_tc50_set_logon_ans'
      , 'sp_tc50_set_logon_req'
      , 'sp_tc50_set_sc_ans'
      , 'sp_tc50_set_sc_req'
      , 'sp_tc50_set_trade_ans'
      , 'sp_tc50_set_trade_req'
      )
  ; 
  IF mysql_err_code <> 0 OR @sp_count_real <> @sp_count THEN
    IF mysql_err_code <> 0 THEN
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1329;
    END IF;

    SET err_msg = 'check sp error';
    SELECT err_no, err_msg;
    LEAVE sys_check_system_proc;
  END IF;

  -- ok
  SET err_no = 0;
  SELECT err_no, err_msg;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_get_rpc
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_get_rpc`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_get_rpc_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- update
  UPDATE t_sys_rpc_invoke
  SET `process_status` = 'GET'
  WHERE `process_status` = 'REQUEST';
    
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Update RPC Failed.');
    LEAVE sys_get_rpc_proc;
  END IF;

  SELECT `id`, `from`, `target`, `name`, `param`
  FROM t_sys_rpc_invoke tsri
  WHERE tsri.process_status = 'GET'
  ORDER BY `request` DESC;-- update
  UPDATE t_sys_rpc_invoke
  SET `process_status` = 'PROCESS'
  WHERE `process_status` = 'GET';
    
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Update RPC Failed.');
    LEAVE sys_get_rpc_proc;
  END IF;-- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_get_setting
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_get_setting`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
get_sys_setting_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  SELECT `section`, `name`, `value`
  FROM t_sys_settings;
  IF mysql_err_code <> 0 || FOUND_ROWS() < 1 THEN
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1364;
    END IF;
    
    SET err_msg = CONCAT('Read Setting Error.');
    LEAVE get_sys_setting_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_log
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_log`(IN `v_type` VARCHAR(8), IN `v_from` VARCHAR(64), IN `v_extern_data` VARCHAR(128), IN `v_content` BLOB, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_log_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- check setting
  SET @log_table_name = CONCAT('t_log_', v_type);
  SELECT value INTO @log_value
  FROM t_sys_settings tss
  WHERE tss.section = 'LOG' AND tss.name = v_type;
  IF FOUND_ROWS() <= 0 OR mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Setting not found.', @log_table_name);
    LEAVE sys_log_proc;
  END IF;
  
  IF @log_value <> 'Y' THEN
    SET err_no = 0;
    SET err_msg = CONCAT('Setting is disable.', @log_table_name);
    LEAVE sys_log_proc;
  END IF;-- get connect info
 
  INSERT INTO t_sys_log 
    (`log_type`, `from`, `extern_data`, `content`)
  VALUES
    (v_type, v_from, v_extern_data, v_content);

  IF ROW_COUNT() <> 1 OR mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'insert into log table failed';
    LEAVE sys_log_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_set_checkpoint
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_set_checkpoint`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_set_checkpoint_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  CALL sp_sys_log('INFO', 'Aggregator', 'CheckPoint', CURRENT_TIMESTAMP(), err_no, err_msg);
  IF mysql_err_code <> 0 OR err_no <> 0 THEN
    IF mysql_err_code <> 0 THEN
      SET err_no = mysql_err_code;
    END IF;
    SET err_msg = CONCAT('Set CheckPoint Error.');
    LEAVE sys_set_checkpoint_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_set_resource_status
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_set_resource_status`(IN `v_name` VARCHAR(64), IN `v_current_size` BIGINT, IN `v_free_size` BIGINT, IN `v_last_alive_time` INT, IN `v_last_err_no` INT, IN `v_last_err_msg` VARCHAR(128), IN `v_info` VARCHAR(256), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_set_resource_status_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  SET @last_alive_time = FROM_UNIXTIME(v_last_alive_time);
  
  INSERT INTO t_sys_resource_status
    (`name`, `current_size`, `free_size`, `last_alive_time`, `last_err_no`, `last_err_msg`, `info`)
  VALUES
    (v_name, v_current_size, v_free_size
    , @last_alive_time, v_last_err_no, v_last_err_msg, v_info)
  ON DUPLICATE KEY UPDATE
    `current_size` = v_current_size
    , `free_size` = v_free_size, `last_alive_time` = @last_alive_time
    , `last_err_no` = v_last_err_no, `last_err_msg` = v_last_err_msg
    , `info` = v_info
  ;
  
  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Set System Resource Status Error.');
    LEAVE sys_set_resource_status_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_sys_set_rpc
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_sys_set_rpc`(IN `v_id` INT, IN `v_err_no` INT, IN `v_err_msg` VARCHAR(512), IN `v_result` BLOB, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
sys_set_rpc_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- update
  UPDATE t_sys_rpc_invoke
  SET `process_status` = 'FINISH'
    , `err_no`  = v_err_no
    , `err_msg` = v_err_msg
    , `result`  = v_result
  WHERE `id` = v_id;

  IF mysql_err_code <> 0 THEN
    SET err_no = mysql_err_code;
    SET err_msg = CONCAT('Update RPC Failed.');
    LEAVE sys_set_rpc_proc;
  END IF;
 
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_get_log_rule
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_get_log_rule`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_get_log_rule:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  SELECT `id` 
  FROM t_tc50_consts
  WHERE `parentid` = 40 AND `log` = 1 AND `id` < 65536
  ORDER BY id;
  
  IF mysql_err_code <> 0 OR FOUND_ROWS() < 1 THEN  
    IF mysql_err_code <> 0 THEN
      SELECT NULL;
      SET err_no = mysql_err_code;
    ELSE
      SET err_no = 1364;            
    END IF;    
    SET err_msg = CONCAT('Get TC50 Log Const Error.', v_collector);
    LEAVE tc50_get_log_rule;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_failed_log
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_failed_log`(IN `v_log_date` INT, IN `v_parser` VARCHAR(64), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_failed_log_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- insert
  INSERT DELAYED INTO t_tc50_log_failed
    (`log_date`, `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`)
  VALUES
    (DATEDIFF(v_log_date,19900101)
    , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Failed LOG Error. t_tc50_log_failed';
    LEAVE tc50_set_failed_log_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_info_log
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_info_log`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_info` VARCHAR(256), IN `v_channel_id` INT, IN `v_ip` VARCHAR(64), IN `v_op_organization` VARCHAR(256), IN `v_op_account` VARCHAR(128), IN `v_reason` VARCHAR(256), IN `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_info_log_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers
  
  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_log_info
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`
      , `info`
      , `channel_id`, `ip`, `op_organization`, `op_account`, `reason`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time
     , v_info
     , v_channel_id, @v_ip, v_op_organization, v_op_account, v_reason
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Info LOG Error. t_tc50_log_info';
    LEAVE tc50_set_info_log_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_logon_ans
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_logon_ans`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_timeA` INT, IN `v_timeB` INT, IN `v_queue` INT, IN `v_return_no` INT, IN `v_return_msg` VARCHAR(512), IN `v_reserve_a` VARCHAR(512), IN `v_reserve_b` VARCHAR(512), IN `v_reserve_c` VARCHAR(512), IN `v_reserve_d` VARCHAR(512), IN `v_reserve_e` VARCHAR(512), IN `v_reserve_f` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_logon_ans_proc:
BEGIN
  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_logon_ans
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `timeA`, `timeB`, `queue`, `return_no`, `return_msg`
      , `reserve_a`, `reserve_b`, `reserve_c`
    	, `reserve_d`, `reserve_e`, `reserve_f`
      )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name
      
     , v_timeA, v_timeB, v_queue, v_return_no, v_return_msg
     , v_reserve_a, v_reserve_b, v_reserve_c
     , v_reserve_d, v_reserve_e, v_reserve_f
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Ans LOG Error. t_tc50_logon_ans';
    LEAVE tc50_set_logon_ans_proc;
  END IF;
    
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_logon_req
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_logon_req`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_XT_GTLB` VARCHAR(8), IN `v_KHH` VARCHAR(32), IN `v_KHMC` VARCHAR(32), IN `v_ZHLB` INT, IN `v_ZJZH` VARCHAR(32), IN `v_SHGD` VARCHAR(32), IN `v_SZGD` VARCHAR(32), IN `v_XT_CLITYPE` INT, IN `v_XT_CLIVER` VARCHAR(8), IN `v_XT_VIPFLAG` INT, IN `v_XT_MACHINEINFO` VARCHAR(1024), IN `v_reserve_a` VARCHAR(512), IN `v_reserve_b` VARCHAR(512), IN `v_reserve_c` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_logon_req_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_logon_req
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `XT_GTLB`, `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `SHGD`, `SZGD`, `XT_CLITYPE`, `XT_CLIVER`, `XT_VIPFLAG`, `XT_MACHINEINFO`
      , `reserve_a`, `reserve_b`, `reserve_c`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name

     , v_XT_GTLB, v_KHH, v_KHMC, v_ZHLB, v_ZJZH, v_SHGD, v_SZGD, v_XT_CLITYPE, v_XT_CLIVER, v_XT_VIPFLAG, v_XT_MACHINEINFO
     , v_reserve_a, v_reserve_b, v_reserve_c
    )
  ;
  
  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Logon Req LOG Error. t_tc50_logon_req';
    LEAVE tc50_set_logon_req_proc;
  END IF;
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_sc_ans
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_sc_ans`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_timeA` INT, IN `v_timeB` INT, IN `v_queue` INT, IN `v_return_no` INT, IN `v_return_msg` VARCHAR(512), IN `v_reserve_a` VARCHAR(512), IN `v_reserve_b` VARCHAR(512), IN `v_reserve_c` VARCHAR(512), IN `v_reserve_d` VARCHAR(512), IN `v_reserve_e` VARCHAR(512), IN `v_reserve_f` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_sc_ans_proc:
BEGIN
  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_sc_ans
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `timeA`, `timeB`, `queue`, `return_no`, `return_msg`
      , `reserve_a`, `reserve_b`, `reserve_c`
    	, `reserve_d`, `reserve_e`, `reserve_f`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name
      
     , v_timeA, v_timeB, v_queue, v_return_no, v_return_msg
     , v_reserve_a, v_reserve_b, v_reserve_c
     , v_reserve_d, v_reserve_e, v_reserve_f
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Ans LOG Error. t_tc50_sc_ans';
    LEAVE tc50_set_sc_ans_proc;
  END IF;
    
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_sc_req
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_sc_req`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` VARCHAR(64), IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_XT_GTLB` VARCHAR(8), IN `v_CA_KHH` VARCHAR(64), IN `v_CA_KHMC` VARCHAR(64), IN `v_CA_VER` VARCHAR(64), IN `v_CA_AQJB` VARCHAR(64), IN `v_CA_TXMM` VARCHAR(64), IN `v_CA_ISVIPHOST` VARCHAR(64), IN `v_CA_JQTZM` VARCHAR(64), IN `v_CA_SLOTSN` VARCHAR(64), IN `v_CA_CID` VARCHAR(64), IN `v_CA_CERTREQ` VARCHAR(64), IN `v_CA_USERCERDN` VARCHAR(64), IN `v_CA_ZSQSRQ` VARCHAR(64), IN `v_CA_ZSJZRQ` VARCHAR(64), IN `v_CA_CERTSN` VARCHAR(64), IN `v_CA_CERTINFO` VARCHAR(64), IN `v_CA_MACHINENAME` VARCHAR(64), IN `v_CA_DLSJ` VARCHAR(64), IN `v_CA_LASTIP` VARCHAR(64), IN `v_CA_MAC` VARCHAR(64), IN `v_CA_CSCS` VARCHAR(64), IN `v_CA_RESV` VARCHAR(64), IN `v_reserve_a` VARCHAR(512), IN `v_reserve_b` VARCHAR(512), IN `v_reserve_c` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_sc_req_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions

  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_sc_req
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `XT_GTLB`, `CA_KHH`, `CA_KHMC`, `CA_VER`, `CA_AQJB`
      , `CA_TXMM`, `CA_ISVIPHOST`, `CA_JQTZM`, `CA_SLOTSN`, `CA_CID`
      , `CA_CERTREQ`, `CA_USERCERDN`, `CA_ZSQSRQ`, `CA_ZSJZRQ`, `CA_CERTSN`
      , `CA_CERTINFO`, `CA_MACHINENAME`, `CA_DLSJ`, `CA_LASTIP`, `CA_MAC`, `CA_CSCS`, `CA_RESV`
      , `reserve_a`, `reserve_b`, `reserve_c`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name

     , v_XT_GTLB, v_CA_KHH, v_CA_KHMC, v_CA_VER, v_CA_AQJB
     , v_CA_TXMM, v_CA_ISVIPHOST, v_CA_JQTZM, v_CA_SLOTSN, v_CA_CID
     , v_CA_CERTREQ, v_CA_USERCERDN, v_CA_ZSQSRQ, v_CA_ZSJZRQ, v_CA_CERTSN
     , v_CA_CERTINFO, v_CA_MACHINENAME, v_CA_DLSJ, v_CA_LASTIP, v_CA_MAC, v_CA_CSCS, v_CA_RESV
     , v_reserve_a, v_reserve_b, v_reserve_c
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert SC Req LOG Error. t_tc50_sc_req';
    LEAVE tc50_set_sc_req_proc;
  END IF;

  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_trade_ans
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_trade_ans`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_timeA` INT, IN `v_timeB` INT, IN `v_queue` INT, IN `v_return_no` INT, IN `v_return_msg` VARCHAR(512), IN `v_WTBH` VARCHAR(32), IN `v_XT_CHECKRISKFLAG` INT, IN `v_RETINFO` VARCHAR(512), IN `v_reserve_d` VARCHAR(512), IN `v_reserve_e` VARCHAR(512), IN `v_reserve_f` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_trade_ans_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;  
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions
  -- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;

  -- insert
  INSERT DELAYED INTO t_tc50_trade_ans
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `timeA`, `timeB`, `queue`, `return_no`, `return_msg`
      , `WTBH`, `XT_CHECKRISKFLAG`, `RETINFO`
      , `reserve_d`, `reserve_e`, `reserve_f`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name
      
     , v_timeA, v_timeB, v_queue, v_return_no, v_return_msg
     , v_WTBH, v_XT_CHECKRISKFLAG, v_RETINFO
     , v_reserve_d, v_reserve_e, v_reserve_f
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Ans LOG Error. t_tc50_trade_ans';
    LEAVE tc50_set_trade_ans_proc;
  END IF;
    
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_set_trade_req
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_set_trade_req`(IN `v_log_date` INT, IN `v_parser` VARCHAR(16), IN `v_collector` VARCHAR(32), IN `v_file_id` INT, IN `v_file_pos_start` BIGINT, IN `v_log_len` INT, IN `v_log_type` VARCHAR(16), IN `v_log_time` INT, IN `v_ip` VARCHAR(64), IN `v_mac` VARCHAR(16), IN `v_thread_id` VARCHAR(16), IN `v_channel_id` INT, IN `v_trans_id` INT, IN `v_req_type` TINYINT, IN `v_func_id` SMALLINT, IN `v_func_name` VARCHAR(96), IN `v_branch_id` INT, IN `v_branch_name` VARCHAR(96), IN `v_KHH` VARCHAR(16), IN `v_KHMC` VARCHAR(32), IN `v_ZHLB` INT, IN `v_ZJZH` VARCHAR(16), IN `v_GDDM` VARCHAR(16), IN `v_OP_WTFS` INT, IN `v_WTBH` VARCHAR(32), IN `v_WTFS` INT, IN `v_ZQDM` VARCHAR(16), IN `v_MMBZ` INT, IN `v_JYDW` INT, IN `v_WTSL` INT, IN `v_WTJG` VARCHAR(16), IN `v_WTRQ` INT, IN `v_WTSJ` INT, IN `v_XT_CHECKRISKFLAG` INT, IN `v_reserve_a` VARCHAR(512), IN `v_reserve_b` VARCHAR(512), IN `v_reserve_c` VARCHAR(512), OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
tc50_set_trade_req_proc:
BEGIN

  -- START Declare Conditions
  DECLARE mysql_err_code INT DEFAULT 0;
  DECLARE no_data CONDITION FOR 1329;
  DECLARE no_define CONDITION FOR 1364;
  DECLARE duplicate_key CONDITION FOR 1062;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION set mysql_err_code = 99999;
  -- END Declare COnditions-- START Declare Exception Handlers
  DECLARE CONTINUE HANDLER FOR duplicate_key
  BEGIN
    SET mysql_err_code = 1062;
  END;

  DECLARE CONTINUE HANDLER FOR no_data
  BEGIN
    SET mysql_err_code = 1329;
  END;

  DECLARE CONTINUE HANDLER FOR no_define
  BEGIN
    SET mysql_err_code = 1364;
  END;
  -- END Declare Exception Handlers

  -- date preprocess
  SET @v_ip               = INET_ATON(v_ip);
  IF @v_ip is NULL THEN
    SET @v_ip               = 0;
  END IF;
  
  SET @v_mac              = CONV(v_mac, 16, 10);
  IF @v_mac is NULL THEN
    SET @v_mac               = 0;
  END IF;

  SET @v_thread_id        = CONV(v_thread_id, 16, 10);
  IF @v_thread_id is NULL THEN
    SET @v_thread_id      	= 0;
  END IF;
  
  SET @v_WTJG           = CONVERT(v_WTJG, DECIMAL(10,3));
  IF @v_WTJG is NULL THEN
    SET @v_WTJG           = 0.000;
  END IF;
  
  -- insert
  INSERT DELAYED INTO t_tc50_trade_req
    (`log_date`
      , `parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`
      , `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`
      
      , `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `GDDM`, `OP_WTFS`
      , `WTBH`, `WTFS`, `ZQDM`, `MMBZ`, `JYDW`, `WTSL`, `WTJG`, `WTRQ`, `WTSJ`
			, `XT_CHECKRISKFLAG`, `reserve_a`, `reserve_b`, `reserve_c`
    )
  VALUES
    (DATEDIFF(v_log_date,19900101)
     , v_parser, v_collector, v_file_id, v_file_pos_start, v_log_len, UNIX_TIMESTAMP()
     , v_log_type, v_log_time, @v_ip, @v_mac, @v_thread_id, v_channel_id, v_trans_id, v_req_type, v_func_id, v_func_name, v_branch_id, v_branch_name

     , v_KHH, v_KHMC, v_ZHLB, v_ZJZH, v_GDDM, v_OP_WTFS
     , v_WTBH, v_WTFS, v_ZQDM, v_MMBZ, v_JYDW, v_WTSL, @v_WTJG, v_WTRQ, v_WTSJ
     , v_XT_CHECKRISKFLAG, v_reserve_a, v_reserve_b, v_reserve_c
    )
  ;

  IF mysql_err_code <> 0 OR ROW_COUNT() <> 1 THEN
    SET err_no = mysql_err_code;
    SET err_msg = 'Insert Req LOG Error. t_tc50_trade_req';
    LEAVE tc50_set_trade_req_proc;
  END IF;
  
  -- ok
  SET err_no = 0;
END//
DELIMITER ;


-- Dumping structure for table tls.t_collector_config
CREATE TABLE IF NOT EXISTS `t_collector_config` (
  `collector` varchar(32) NOT NULL,
  `config_no` int(10) unsigned NOT NULL,
  `start` int(10) unsigned NOT NULL COMMENT 'link 223015',
  `end` int(10) unsigned NOT NULL,
  `steptime` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Sec',
  `keepalive` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Sec',
  `rate` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'KB/S',
  PRIMARY KEY (`collector`,`config_no`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_collector_config: 2 rows
/*!40000 ALTER TABLE `t_collector_config` DISABLE KEYS */;
INSERT INTO `t_collector_config` (`collector`, `config_no`, `start`, `end`, `steptime`, `keepalive`, `rate`) VALUES
	(' default', 1, 80000, 153000, 0, 60, 64),
	(' default', 2, 153000, 60000, 5, 60, 512);
/*!40000 ALTER TABLE `t_collector_config` ENABLE KEYS */;


-- Dumping structure for table tls.t_collector_file
CREATE TABLE IF NOT EXISTS `t_collector_file` (
  `id` int(10) unsigned NOT NULL COMMENT '由collector提交文件名,Aggregator生成全局唯一',
  `status` enum('PROCESS','FINISH') NOT NULL DEFAULT 'PROCESS',
  `add_time` timestamp NULL DEFAULT NULL,
  `last_update` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `collector` varchar(96) NOT NULL COMMENT 'IP_MAC',
  `file_size` bigint(20) unsigned NOT NULL DEFAULT '0',
  `create_time` bigint(20) unsigned NOT NULL DEFAULT '0',
  `collector_dir` varchar(256) NOT NULL,
  `collector_name` varchar(61) NOT NULL,
  `aggregator_path` varchar(256) NOT NULL DEFAULT '',
  `checksum` varchar(64) NOT NULL DEFAULT '',
  `block_checksum` mediumblob COMMENT '4k ==> crc32',
  PRIMARY KEY (`id`),
  UNIQUE KEY `aggregator_path` (`aggregator_path`),
  KEY `collector` (`collector`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_collector_file: 0 rows
/*!40000 ALTER TABLE `t_collector_file` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_collector_file` ENABLE KEYS */;


-- Dumping structure for table tls.t_collector_rule
CREATE TABLE IF NOT EXISTS `t_collector_rule` (
  `collector` varchar(32) NOT NULL,
  `rule_no` smallint(5) unsigned NOT NULL,
  `judge_rule` set('PATH','FILENAME') NOT NULL COMMENT '判断规则,同时满足条件',
  `dir` varchar(512) NOT NULL,
  `exclude` varchar(512) NOT NULL,
  `include` varchar(512) NOT NULL,
  `comment` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`collector`,`rule_no`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_collector_rule: 3 rows
/*!40000 ALTER TABLE `t_collector_rule` DISABLE KEYS */;
INSERT INTO `t_collector_rule` (`collector`, `rule_no`, `judge_rule`, `dir`, `exclude`, `include`, `comment`) VALUES
	(' default', 0, 'PATH,FILENAME', 'f:/tdxlog/test/日志文件', '', '^20[0-9]{6}\\.log$', '搜集目录下的所有log文件'),
	(' default', 1, 'PATH', 'z:/abcd', '', '[a-z]*\\.h$', ''),
	(' default', 2, 'PATH,FILENAME', 'z:/o', '*\\.7z$', '^20[0-9]{6}\\.log$', '');
/*!40000 ALTER TABLE `t_collector_rule` ENABLE KEYS */;


-- Dumping structure for table tls.t_collector_status
CREATE TABLE IF NOT EXISTS `t_collector_status` (
  `collector` varchar(32) NOT NULL,
  `send_data_len` bigint(20) unsigned NOT NULL,
  `recv_data_len` bigint(20) unsigned NOT NULL,
  `recv_data_len_real` bigint(20) unsigned NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `logon_time` int(10) unsigned NOT NULL,
  `last_keepalive` int(10) unsigned NOT NULL,
  PRIMARY KEY (`collector`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_collector_status: 0 rows
/*!40000 ALTER TABLE `t_collector_status` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_collector_status` ENABLE KEYS */;


-- Dumping structure for table tls.t_module_setting
CREATE TABLE IF NOT EXISTS `t_module_setting` (
  `module` enum('ParserTC50','UnKnown') NOT NULL,
  `name` int(10) unsigned NOT NULL,
  `value` bigint(20) NOT NULL,
  `data` blob NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`module`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_module_setting: 0 rows
/*!40000 ALTER TABLE `t_module_setting` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_module_setting` ENABLE KEYS */;


-- Dumping structure for table tls.t_parser_logtype_rule
CREATE TABLE IF NOT EXISTS `t_parser_logtype_rule` (
  `type` varchar(64) NOT NULL,
  `file_dir` varchar(256) NOT NULL DEFAULT '' COMMENT '文件路径(正则表达式)',
  `file_name` varchar(64) NOT NULL COMMENT '文件路径(正则表达式)',
  `file_head` blob,
  `judge_rule` set('DIR','NAME','HEAD') NOT NULL DEFAULT 'NAME',
  `comment` varchar(128) NOT NULL DEFAULT '',
  `encode_type` char(8) NOT NULL DEFAULT '',
  `encode_param` varchar(64) NOT NULL DEFAULT '',
  `checksum_type` varchar(16) NOT NULL DEFAULT '' COMMENT '[md5,crc32],[md5,adler32], [全文,分块]',
  `checksum_param` varchar(64) NOT NULL DEFAULT '' COMMENT '1K,4k(分块校验大小)',
  KEY `type` (`type`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_parser_logtype_rule: 2 rows
/*!40000 ALTER TABLE `t_parser_logtype_rule` DISABLE KEYS */;
INSERT INTO `t_parser_logtype_rule` (`type`, `file_dir`, `file_name`, `file_head`, `judge_rule`, `comment`, `encode_type`, `encode_param`, `checksum_type`, `checksum_param`) VALUES
	('LT_TC50', '', '^20[0-9]{6}\\.log$', NULL, 'NAME', 'TC50 交易日志', '', '', '', ''),
	('LT_TC50', '', '[0-9a-zA-Z.]*\\.sto$', NULL, 'NAME', 'TC50 字典文件', '', '', '', '');
/*!40000 ALTER TABLE `t_parser_logtype_rule` ENABLE KEYS */;


-- Dumping structure for table tls.t_sys_log
CREATE TABLE IF NOT EXISTS `t_sys_log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `log_type` char(8) NOT NULL,
  `from` varchar(32) NOT NULL,
  `extern_data` varchar(128) NOT NULL DEFAULT '',
  `process_status` int(10) unsigned NOT NULL DEFAULT '0',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `content` varchar(256) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_sys_log: 0 rows
/*!40000 ALTER TABLE `t_sys_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_sys_log` ENABLE KEYS */;


-- Dumping structure for table tls.t_sys_resource_status
CREATE TABLE IF NOT EXISTS `t_sys_resource_status` (
  `name` varchar(32) NOT NULL,
  `current_size` bigint(20) unsigned NOT NULL,
  `free_size` bigint(20) unsigned NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `last_alive_time` int(10) unsigned NOT NULL,
  `last_err_no` int(10) unsigned NOT NULL,
  `last_err_msg` varchar(128) NOT NULL,
  `info` char(255) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_sys_resource_status: 0 rows
/*!40000 ALTER TABLE `t_sys_resource_status` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_sys_resource_status` ENABLE KEYS */;


-- Dumping structure for table tls.t_sys_rpc_invoke
CREATE TABLE IF NOT EXISTS `t_sys_rpc_invoke` (
  `id` int(10) unsigned NOT NULL,
  `from` varchar(64) NOT NULL,
  `target` varchar(16) NOT NULL,
  `name` varchar(16) NOT NULL,
  `param` blob,
  `process_status` enum('REQUEST','GET','PROCESS','FINISH') DEFAULT 'REQUEST',
  `result` blob,
  `err_no` int(10) unsigned DEFAULT NULL,
  `err_msg` varchar(512) DEFAULT NULL,
  `request` timestamp NULL DEFAULT NULL,
  `reply` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_sys_rpc_invoke: 0 rows
/*!40000 ALTER TABLE `t_sys_rpc_invoke` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_sys_rpc_invoke` ENABLE KEYS */;


-- Dumping structure for table tls.t_sys_settings
CREATE TABLE IF NOT EXISTS `t_sys_settings` (
  `section` varchar(64) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(64) NOT NULL,
  `comment` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`section`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_sys_settings: 22 rows
/*!40000 ALTER TABLE `t_sys_settings` DISABLE KEYS */;
INSERT INTO `t_sys_settings` (`section`, `name`, `value`, `comment`) VALUES
	('LOG', 'TRACE', 'Y', '跟踪日志(开启)'),
	('LOG', 'ERR', 'Y', '错误日志(开启)'),
	('LOG', 'INFO', 'Y', '信息日志(开启)'),
	('LOG', 'ALARM', 'Y', '警报日志(开启)'),
	('AGGREGATOR', 'StepTimeRPCScan', '300', 'rpc_invoke 表扫描间隔(秒)'),
	('AGGREGATOR', 'CollectorFileLocalDir', '20130715', '采集文件 本地存储目录'),
	('AGGREGATOR', 'Name', 'tdxAgg', '汇聚器名称'),
	('AGGREGATOR', 'StepTimeResourceStatus', '600', '资源使用间隔(秒)'),
	('AGGREGATOR', 'StepTimeClientStatus', '60', '采集器状态间隔(秒)'),
	('LOG', 'WARN', 'Y', '警告日志'),
	('AGGREGATOR', 'StepTimeUpdateFile', '1800', '更新文件信息间隔(秒)'),
	('AGGREGATOR', 'StepTimeCheckPoint', '3600', 'CheckPoint间隔(秒)'),
	('ParserTC50', 'ThreadNum', '3', '解析线程数'),
	('ParserTC50', 'ScheduleTime', '10000', '实时转换历史时间(凌晨1点)'),
	('AGGREGATOR', 'FileSystemCacheSize', '256', '文件系统缓存大小(M)'),
	('ParserTC50', 'HisDBName', 'tls_his', '历史库名'),
	('ParserTC50', 'HisDBRemote', 'Y', '远程数据库(修改前请咨询)'),
	('ParserTC50', 'HisDBEng', 'BDE', '历史库数据引擎(修改前请咨询)'),
	('AGGREGATOR', 'WakeupParserTime', '300', '唤醒解析器时间间隔(毫秒)'),
	('AGGREGATOR', 'LongTickTime', '60000', '长时钟间隔(毫秒)'),
	('ParserTC50', 'TransformRows', '1048576', '转历史库表记录条数限制(1M条)'),
	('ParserTC50', 'ScheduleEnable', 'Y', '是否进行历史库转换');
/*!40000 ALTER TABLE `t_sys_settings` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_consts
CREATE TABLE IF NOT EXISTS `t_tc50_consts` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(128) NOT NULL,
  `descript` varchar(128) NOT NULL DEFAULT '',
  `parentid` int(10) unsigned NOT NULL,
  `isvalid` int(10) unsigned NOT NULL DEFAULT '1',
  `log` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '是否记录日志',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  UNIQUE KEY `id_parentid` (`id`,`parentid`),
  KEY `log` (`log`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_consts: 105 rows
/*!40000 ALTER TABLE `t_tc50_consts` DISABLE KEYS */;
INSERT INTO `t_tc50_consts` (`id`, `name`, `descript`, `parentid`, `isvalid`, `log`, `timestamp`) VALUES
	(94, '账号预处理', '功能名称', 40, 1, 1, '2013-08-27 10:48:16'),
	(96, '登录预处理', '功能名称', 40, 1, 1, '2013-08-27 10:44:42'),
	(98, '集成客户校验', '功能名称', 40, 1, 1, '2013-08-27 10:44:42'),
	(100, '客户校验', '功能名称', 40, 1, 1, '2013-08-27 10:44:42'),
	(202, '普通股票委托', '功能名称', 40, 1, 1, '2013-10-31 16:20:07');
/*!40000 ALTER TABLE `t_tc50_consts` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_logon_ans
CREATE TABLE IF NOT EXISTS `t_tc50_logon_ans` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) unsigned NOT NULL,
  `branch_name` varchar(64) NOT NULL,
  `timeA` int(10) unsigned NOT NULL,
  `timeB` int(10) unsigned NOT NULL,
  `queue` int(10) unsigned NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_logon_ans: 0 rows
/*!40000 ALTER TABLE `t_tc50_logon_ans` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_logon_ans` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_logon_req
CREATE TABLE IF NOT EXISTS `t_tc50_logon_req` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) NOT NULL,
  `branch_name` varchar(64) NOT NULL,
  `XT_GTLB` char(2) NOT NULL,
  `KHH` varchar(16) NOT NULL,
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
  `reserve_c` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_logon_req: 0 rows
/*!40000 ALTER TABLE `t_tc50_logon_req` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_logon_req` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_log_failed
CREATE TABLE IF NOT EXISTS `t_tc50_log_failed` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL,
  `collector` varchar(32) NOT NULL,
  `file_id` int(10) unsigned NOT NULL,
  `file_pos_start` bigint(20) unsigned NOT NULL,
  `log_len` int(10) unsigned NOT NULL,
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_log_failed: 0 rows
/*!40000 ALTER TABLE `t_tc50_log_failed` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_log_failed` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_log_info
CREATE TABLE IF NOT EXISTS `t_tc50_log_info` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL COMMENT '解析者',
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `info` varchar(64) NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `op_organization` varchar(64) NOT NULL,
  `op_account` varchar(32) NOT NULL,
  `reason` varchar(64) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_log_info: 0 rows
/*!40000 ALTER TABLE `t_tc50_log_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_log_info` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_sc_ans
CREATE TABLE IF NOT EXISTS `t_tc50_sc_ans` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) unsigned NOT NULL,
  `branch_name` varchar(64) NOT NULL,
  `timeA` int(10) unsigned NOT NULL,
  `timeB` int(10) unsigned NOT NULL,
  `queue` int(10) unsigned NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `reserve_a` varchar(512) NOT NULL,
  `reserve_b` varchar(512) NOT NULL,
  `reserve_c` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_sc_ans: 0 rows
/*!40000 ALTER TABLE `t_tc50_sc_ans` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_sc_ans` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_sc_req
CREATE TABLE IF NOT EXISTS `t_tc50_sc_req` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) unsigned NOT NULL,
  `branch_name` varchar(64) NOT NULL,
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
  `reserve_c` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_sc_req: 0 rows
/*!40000 ALTER TABLE `t_tc50_sc_req` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_sc_req` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_trade_ans
CREATE TABLE IF NOT EXISTS `t_tc50_trade_ans` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) unsigned NOT NULL,
  `branch_name` varchar(64) NOT NULL,
  `timeA` int(10) unsigned NOT NULL,
  `timeB` int(10) unsigned NOT NULL,
  `queue` int(10) unsigned NOT NULL,
  `return_no` int(10) NOT NULL,
  `return_msg` varchar(512) NOT NULL,
  `WTBH` varchar(32) NOT NULL,
  `XT_CHECKRISKFLAG` int(11) NOT NULL,
  `RETINFO` varchar(512) NOT NULL,
  `reserve_d` varchar(512) NOT NULL,
  `reserve_e` varchar(512) NOT NULL,
  `reserve_f` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_trade_ans: 0 rows
/*!40000 ALTER TABLE `t_tc50_trade_ans` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_trade_ans` ENABLE KEYS */;


-- Dumping structure for table tls.t_tc50_trade_req
CREATE TABLE IF NOT EXISTS `t_tc50_trade_req` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `parser` enum('ParserTC50','UnKnown') NOT NULL COMMENT '解析者',
  `collector` varchar(32) NOT NULL COMMENT '采集者',
  `file_id` int(10) unsigned NOT NULL COMMENT '文件编号',
  `file_pos_start` bigint(20) unsigned NOT NULL COMMENT '文件位置',
  `log_len` int(10) unsigned NOT NULL COMMENT '文件位置',
  `timestamp` int(10) unsigned NOT NULL COMMENT '添加时间',
  `log_date` smallint(5) unsigned NOT NULL,
  `log_type` enum('系统信息','系统错误','连接建立','连接断开','功能请求','成功处理','调用失败','协议异常','处理异常','通讯关闭','UnKnown') NOT NULL,
  `log_time` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mac` bigint(20) unsigned NOT NULL,
  `thread_id` smallint(5) unsigned NOT NULL,
  `channel_id` int(10) unsigned NOT NULL,
  `trans_id` int(10) unsigned NOT NULL,
  `req_type` tinyint(3) unsigned NOT NULL,
  `func_id` smallint(5) unsigned NOT NULL,
  `func_name` varchar(32) NOT NULL,
  `branch_id` int(10) unsigned NOT NULL,
  `branch_name` varchar(64) NOT NULL,
  `KHH` varchar(16) NOT NULL,
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
  `reserve_c` varchar(512) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `file_id_file_pos_start` (`file_id`,`file_pos_start`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table tls.t_tc50_trade_req: 0 rows
/*!40000 ALTER TABLE `t_tc50_trade_req` DISABLE KEYS */;
/*!40000 ALTER TABLE `t_tc50_trade_req` ENABLE KEYS */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
