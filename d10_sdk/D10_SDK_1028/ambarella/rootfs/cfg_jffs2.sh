echo "### make cfg.jffs2 ###"

mkfs.jffs2 -p 0x200000 -d cfg \
-e 128KiB -s 2048 -o  cfg.jffs2 -l -n -m priority

echo "### cfg.jffs2 made ###"


