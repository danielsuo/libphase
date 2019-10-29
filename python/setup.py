from setuptools import find_packages, setup

setup(
    name="libphase",
    version="0.0.1",
    install_requires=[
        "numpy",
        "scipy",
        "scikit-learn",
        "ipython",
        "click",
        "clickutil",
        "tqdm",
        "pandas",
        "matplotlib",
        "jupyter",
        "ipynb",
        "pixiedust"
    ],
    author="Daniel Suo",
    author_email="dsuo@cs.princeton.edu",
    description="libphase",
    python_requires=">=3.6",
    packages=find_packages()
)
