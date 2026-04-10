BEGIN {
  FS = ","
  rows = 0
}

NR == 1 {
  if ($1 != "name" || $2 != "psnr_before" || $3 != "psnr_after" ||
      $4 != "ssim_before" || $5 != "ssim_after" || $6 != "status_code") {
    exit 1
  }
  next
}

NF != 6 {
  exit 1
}

{
  for (i = 2; i <= 5; ++i) {
    if ($i !~ /^-?[0-9]+(\.[0-9]+)?$/) {
      exit 1
    }
  }
  if ($6 !~ /^-?[0-9]+$/) {
    exit 1
  }
  rows += 1
}

END {
  if (rows != expected_count) {
    exit 1
  }
}
