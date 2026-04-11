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
  if ($2 !~ /^-?[0-9]+(\.[0-9]+)?$/ || $3 !~ /^-?[0-9]+(\.[0-9]+)?$/) {
    exit 1
  }
  if (($4 == "N/A" && $5 != "N/A") || ($4 != "N/A" && $5 == "N/A")) {
    exit 1
  }
  if ($4 != "N/A" && $4 !~ /^-?[0-9]+(\.[0-9]+)?$/) {
    exit 1
  }
  if ($5 != "N/A" && $5 !~ /^-?[0-9]+(\.[0-9]+)?$/) {
    exit 1
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
