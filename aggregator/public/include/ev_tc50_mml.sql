-- --------------------------------------------------------
-- Host:                         192.168.2.80
-- Server version:               5.1.40 - Source distribution
-- Server OS:                    Win64
-- HeidiSQL version:             7.0.0.4206
-- Date/time:                    2013-10-28 09:17:38
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for event tls.ev_tc50_clear_mml_closing
DELIMITER //
CREATE EVENT `ev_tc50_clear_mml_closing` ON SCHEDULE EVERY 1 DAY STARTS '2013-10-26 15:40:00' ON COMPLETION PRESERVE ENABLE COMMENT '收市后执行' DO BEGIN
	CALL sp_tc50_clear_mml(@err_no, @err_msg);
	CALL sp_sys_log('TRACE', 'EV_TC50', 'CALL sp_tc50_clear_mml', 'closing', @err_no, @err_msg);
END//
DELIMITER ;


-- Dumping structure for event tls.ev_tc50_clear_mml_transfrom
DELIMITER //
CREATE EVENT `ev_tc50_clear_mml_transfrom` ON SCHEDULE EVERY 1 DAY STARTS '2013-10-26 12:30:00' ON COMPLETION PRESERVE ENABLE COMMENT '转历史库之前执行' DO BEGIN
	CALL sp_tc50_clear_mml(@err_no, @err_msg);
	CALL sp_sys_log('TRACE', 'EV_TC50', 'CALL sp_tc50_clear_mml', 'transfrom', @err_no, @err_msg);
END//
DELIMITER ;


-- Dumping structure for procedure tls.sp_tc50_clear_mml
DELIMITER //
CREATE DEFINER=`xjs`@`tdx` PROCEDURE `sp_tc50_clear_mml`(OUT `err_no` INT, OUT `err_msg` VARCHAR(128))
    SQL SECURITY INVOKER
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

	-- clear log
	-- trade
	DELETE FROM t_tc50_trade_req
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	DELETE FROM t_tc50_trade_ans
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	-- sc
	DELETE FROM t_tc50_sc_req
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	DELETE FROM t_tc50_sc_ans
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;

	-- logon
	DELETE FROM t_tc50_logon_req
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	DELETE FROM t_tc50_logon_ans
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	-- info 
	DELETE FROM t_tc50_log_info
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;

	-- failed
	DELETE FROM t_tc50_log_failed
	WHERE `file_id`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	-- mod
	DELETE FROM t_module_setting
	WHERE `name`
	IN
	(
		SELECT `id` FROM t_collector_file
		WHERE `collector_dir` LIKE '%MemMiniLog'
	)
	;
	
	-- file
	DELETE FROM t_collector_file
	WHERE `collector_dir` LIKE '%MemMiniLog'
	;
	-- file
	
	SET err_no = 0;
	SET err_msg = '';
END//
DELIMITER ;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
