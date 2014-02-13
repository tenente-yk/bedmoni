sudo cp ./bedmoni /home/tenente/pxa270/rootfs/rootfs_nfs/usr/local/bin/ -v
sudo cp ./bedmoni /home/tenente/pxa270/rootfs/rootfs_ext2/usr/local/bin/ -v
sudo cp ./bedmoni_data/*.lang /home/tenente/pxa270/rootfs/rootfs_ext2/usr/local/bedmoni_data/ -v
sudo cp ./bedmoni_data/*.lang /home/tenente/pxa270/rootfs/rootfs_nfs/usr/local/bedmoni_data/ -v
sudo mkdir /home/tenente/pxa270/rootfs/rootfs_ext2/usr/local/fonts
sudo cp ./fonts/* /home/tenente/pxa270/rootfs/rootfs_ext2/usr/local/fonts -r
sudo mkdir /home/tenente/pxa270/rootfs/rootfs_nfs/usr/local/fonts
sudo cp ./fonts/* /home/tenente/pxa270/rootfs/rootfs_nfs/usr/local/fonts -r

mkdir /home/tenente/my_projects/bmoni_updater/bedmoni_data
cp ./bedmoni_data/*.lang /home/tenente/my_projects/bmoni_updater/bedmoni_data -v
mkdir /home/tenente/my_projects/bmoni_updater/bin
cp ./bedmoni /home/tenente/my_projects/bmoni_updater/bin -v
cd /home/tenente/my_projects/bmoni_updater
./make_arc.sh
