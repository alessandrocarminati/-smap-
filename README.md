# description

Scan provided symbol table `System.map` or `kallsyms` and reports where 
the provided address falls into.
```
# cat /proc/kallsyms |sort | grep  " ahci_check_ready" -A1 -B1
ffffffffc033d1f0 t ahci_fill_cmd_slot	[libahci]
ffffffffc033d240 t ahci_check_ready	[libahci]
ffffffffc033d290 t ahci_bad_pmp_check_ready	[libahci]
# ./smap  -f /proc/kallsyms -a 0xffffffffc033d268
0xffffffffc033d268 -> ahci_check_ready
#
```
