from setuptools import find_packages, setup
from glob import glob
import os

package_name = 'simple_bot'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    # syntax: (destination_folder, [list of files to copy])
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),

        # INSTALL CONFIG FILES
        (os.path.join('share', package_name, 'config'),
        glob('config/*')),

        # INSTALL DESCRIPTION FILES
        (os.path.join('share', package_name, 'description'),
        glob('description/*')),    

        # INSTALL LAUNCH FILES
        (os.path.join('share', package_name, 'launch'),
            glob('launch/*.launch.py')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='TODO',
    maintainer_email='TODO',
    description='TODO',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'read_lidar = simple_bot.read_lidar:main',
            'compute_icp = simple_bot.compute_icp:main',
        ],
    },
)