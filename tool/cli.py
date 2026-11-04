import argparse
import os
import pack
import build
import setup


def main():
    parser = argparse.ArgumentParser(
        prog='build',
        description="Shader Builder: Wrapper for Lazurite, to build Bedralis Shader for RenderDragon",
        epilog="GitHub: https://github.com/gby-kbr/Bedralis-Shader"
    )

    profiles = ['android', 'windows', 'ios', 'multiplatform']
    profile_default = profiles[1 if os.name == 'nt' else 0]

    subparsers = parser.add_subparsers(help='sub-command', dest='subcommand')
    pack_parser = subparsers.add_parser('pack', help="build shader pack")
    pack_parser.set_defaults(func=pack.run)
    mats_parser = subparsers.add_parser('mats', help="build shader materials")
    mats_parser.set_defaults(func=build.run)
    setup_parser = subparsers.add_parser('setup', help="setup build tool")
    setup_parser.set_defaults(func=setup.run)

    for p in [pack_parser, mats_parser]:
        p.add_argument(
            '-p',
            choices=profiles,
            default=profile_default,
            help='build profile'
        )

    mats_parser.add_argument(
        '-m',
        default=[],
        nargs='+',
        help="build materials (eg: Sky)"
    )

    mats_parser.add_argument(
        '-s',
        default="",
        help="macro to define while building (eg: DEBUG)"
    )

    setup_parser.add_argument(
        '--reset',
        action='store_true',
        help="reset all data"
    )

    pack_parser.add_argument(
        '--no-zip',
        action='store_true',
        help="don't make archive"
    )

    pack_parser.add_argument(
        '--no-label',
        action='store_true',
        help="don't label materials"
    )

    args = parser.parse_args()

    if args.subcommand is not None:
        args.func(args)
    else:
        parser.print_usage()
