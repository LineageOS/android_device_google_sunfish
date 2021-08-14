#!/vendor/bin/sh

#############################################################
### init.insmod.cfg format:                               ###
### ----------------------------------------------------- ###
### [insmod|setprop|enable/moprobe|wait] [path|prop name] ###
### ...                                                   ###
#############################################################

# imitates wait_for_file() in init
wait_for_file()
{
    filename="${1}"
    timeout="${2:-5}"

    expiry=$(($(date "+%s")+timeout))
    while [[ ! -e "${filename}" ]] && [[ "$(date "+%s")" -le "${expiry}" ]]
    do
        sleep 0.01
    done
}

if [ $# -eq 1 ]; then
  cfg_file=$1
else
  exit 1
fi


if [ -f $cfg_file ]; then
  while IFS="|" read -r action arg
  do
    case $action in
      "insmod") insmod $arg ;;
      "setprop") setprop $arg 1 ;;
      "enable") echo 1 > $arg ;;
      "modprobe")
        case ${arg} in
          "-b *" | "-b")
            arg="-b $(cat /vendor/lib/modules/modules.load)" ;;
          "*" | "")
            arg="$(cat /vendor/lib/modules/modules.load)" ;;
        esac
        modprobe -a -d /vendor/lib/modules $arg ;;
      "wait") wait_for_file $arg ;;
    esac
  done < $cfg_file
fi

# set property even if there is no insmod config
# as property value "1" is expected in early-boot trigger
setprop vendor.all.modules.ready 1
setprop vendor.all.devices.ready 1
