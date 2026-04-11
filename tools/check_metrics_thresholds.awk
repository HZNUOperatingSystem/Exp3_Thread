BEGIN {
  FS = ","
  rows = 0
  psnr_before_sum = 0.0
  psnr_after_sum = 0.0
  ssim_before_sum = 0.0
  ssim_after_sum = 0.0
}

function fail(message) {
  print message > "/dev/stderr"
  exit 1
}

NR == 1 {
  if ($1 != "name" || $2 != "psnr_before" || $3 != "psnr_after" ||
      $4 != "ssim_before" || $5 != "ssim_after" || $6 != "status_code") {
    fail("metrics.csv header is invalid")
  }
  next
}

{
  rows += 1

  if ($6 != "0") {
    fail("metrics.csv contains a non-zero status_code")
  }

  psnr_before_sum += $2 + 0.0
  psnr_after_sum += $3 + 0.0

  if (require_ssim == 1) {
    if ($4 == "N/A" || $5 == "N/A") {
      fail("SSIM columns must be available for this chapter")
    }
    ssim_before_sum += $4 + 0.0
    ssim_after_sum += $5 + 0.0
  } else {
    if ($4 != "N/A" || $5 != "N/A") {
      fail("SSIM columns must be N/A for this chapter")
    }
  }
}

END {
  if (rows != expected_count) {
    fail("metrics.csv row count does not match the dataset")
  }

  avg_psnr_before = psnr_before_sum / rows
  avg_psnr_after = psnr_after_sum / rows

  if (avg_psnr_before < psnr_before_min || avg_psnr_before > psnr_before_max) {
    fail("average psnr_before is outside the expected range")
  }
  if (avg_psnr_after < psnr_after_min || avg_psnr_after > psnr_after_max) {
    fail("average psnr_after is outside the expected range")
  }

  if (require_ssim == 1) {
    avg_ssim_before = ssim_before_sum / rows
    avg_ssim_after = ssim_after_sum / rows

    if (avg_ssim_before < ssim_before_min || avg_ssim_before > ssim_before_max) {
      fail("average ssim_before is outside the expected range")
    }
    if (avg_ssim_after < ssim_after_min || avg_ssim_after > ssim_after_max) {
      fail("average ssim_after is outside the expected range")
    }
  }
}
