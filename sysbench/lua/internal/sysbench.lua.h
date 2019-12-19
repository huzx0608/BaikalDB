unsigned char sysbench_lua[] =
  "-- Copyright (C) 2016-2018 Alexey Kopytov <akopytov@gmail.com>\n"
  "\n"
  "-- This program is free software; you can redistribute it and/or modify\n"
  "-- it under the terms of the GNU General Public License as published by\n"
  "-- the Free Software Foundation; either version 2 of the License, or\n"
  "-- (at your option) any later version.\n"
  "\n"
  "-- This program is distributed in the hope that it will be useful,\n"
  "-- but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  "-- GNU General Public License for more details.\n"
  "\n"
  "-- You should have received a copy of the GNU General Public License\n"
  "-- along with this program; if not, write to the Free Software\n"
  "-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\n"
  "\n"
  "ffi = require(\"ffi\")\n"
  "\n"
  "ffi.cdef[[\n"
  "void sb_event_start(int thread_id);\n"
  "void sb_event_stop(int thread_id);\n"
  "bool sb_more_events(int thread_id);\n"
  "]]\n"
  "\n"
  "-- ----------------------------------------------------------------------\n"
  "-- Main event loop. This is a Lua version of sysbench.c:thread_run()\n"
  "-- ----------------------------------------------------------------------\n"
  "function thread_run(thread_id)\n"
  "   while ffi.C.sb_more_events(thread_id) do\n"
  "      ffi.C.sb_event_start(thread_id)\n"
  "\n"
  "      local success, ret\n"
  "      repeat\n"
  "         success, ret = pcall(event, thread_id)\n"
  "\n"
  "         if not success then\n"
  "            if type(ret) == \"table\" and\n"
  "               ret.errcode == sysbench.error.RESTART_EVENT\n"
  "            then\n"
  "               if sysbench.hooks.before_restart_event then\n"
  "                  sysbench.hooks.before_restart_event(ret)\n"
  "               end\n"
  "            else\n"
  "               error(ret, 2) -- propagate unknown errors\n"
  "            end\n"
  "         end\n"
  "      until success\n"
  "\n"
  "      -- Stop the benchmark if event() returns a value other than nil or false\n"
  "      if ret then\n"
  "         break\n"
  "      end\n"
  "\n"
  "      ffi.C.sb_event_stop(thread_id)\n"
  "   end\n"
  "end\n"
  "\n"
  "-- ----------------------------------------------------------------------\n"
  "-- Hooks\n"
  "-- ----------------------------------------------------------------------\n"
  "\n"
  "sysbench.hooks = {\n"
  "   -- sql_error_ignorable = <func>,\n"
  "   -- report_intermediate = <func>,\n"
  "   -- report_cumulative = <func>\n"
  "}\n"
  "\n"
  "-- Report statistics in the CSV format. Add the following to your\n"
  "-- script to replace the default human-readable reports\n"
  "--\n"
  "-- sysbench.hooks.report_intermediate = sysbench.report_csv\n"
  "function sysbench.report_csv(stat)\n"
  "   local seconds = stat.time_interval\n"
  "   print(string.format(\"%.0f,%u,%4.2f,\" ..\n"
  "                          \"%4.2f,%4.2f,%4.2f,%4.2f,\" ..\n"
  "                          \"%4.2f,%4.2f,\" ..\n"
  "                          \"%4.2f\",\n"
  "                       stat.time_total,\n"
  "                       stat.threads_running,\n"
  "                       stat.events / seconds,\n"
  "                       (stat.reads + stat.writes + stat.other) / seconds,\n"
  "                       stat.reads / seconds,\n"
  "                       stat.writes / seconds,\n"
  "                       stat.other / seconds,\n"
  "                       stat.latency_pct * 1000,\n"
  "                       stat.errors / seconds,\n"
  "                       stat.reconnects / seconds\n"
  "   ))\n"
  "end\n"
  "\n"
  "-- Report statistics in the JSON format. Add the following to your\n"
  "-- script to replace the default human-readable reports\n"
  "--\n"
  "-- sysbench.hooks.report_intermediate = sysbench.report_json\n"
  "function sysbench.report_json(stat)\n"
  "   if not gobj then\n"
  "      io.write('[\\n')\n"
  "      -- hack to print the closing bracket when the Lua state of the reporting\n"
  "      -- thread is closed\n"
  "      gobj = newproxy(true)\n"
  "      getmetatable(gobj).__gc = function () io.write('\\n]\\n') end\n"
  "   else\n"
  "      io.write(',\\n')\n"
  "   end\n"
  "\n"
  "   local seconds = stat.time_interval\n"
  "   io.write(([[\n"
  "  {\n"
  "    \"time\": %4.0f,\n"
  "    \"threads\": %u,\n"
  "    \"tps\": %4.2f,\n"
  "    \"qps\": {\n"
  "      \"total\": %4.2f,\n"
  "      \"reads\": %4.2f,\n"
  "      \"writes\": %4.2f,\n"
  "      \"other\": %4.2f\n"
  "    },\n"
  "    \"latency\": %4.2f,\n"
  "    \"errors\": %4.2f,\n"
  "    \"reconnects\": %4.2f\n"
  "  }]]):format(\n"
  "            stat.time_total,\n"
  "            stat.threads_running,\n"
  "            stat.events / seconds,\n"
  "            (stat.reads + stat.writes + stat.other) / seconds,\n"
  "            stat.reads / seconds,\n"
  "            stat.writes / seconds,\n"
  "            stat.other / seconds,\n"
  "            stat.latency_pct * 1000,\n"
  "            stat.errors / seconds,\n"
  "            stat.reconnects / seconds\n"
  "   ))\n"
  "end\n"
  "\n"
  "-- Report statistics in the default human-readable format. You can use it if you\n"
  "-- want to augment default reports with your own statistics. Call it from your\n"
  "-- own report hook, e.g.:\n"
  "--\n"
  "-- function sysbench.hooks.report_intermediate(stat)\n"
  "--   print(\"my stat: \", val)\n"
  "--   sysbench.report_default(stat)\n"
  "-- end\n"
  "function sysbench.report_default(stat)\n"
  "   local seconds = stat.time_interval\n"
  "   print(string.format(\"[ %.0fs ] thds: %u tps: %4.2f qps: %4.2f \" ..\n"
  "                          \"(r/w/o: %4.2f/%4.2f/%4.2f) lat (ms,%u%%): %4.2f \" ..\n"
  "                          \"err/s %4.2f reconn/s: %4.2f\",\n"
  "                       stat.time_total,\n"
  "                       stat.threads_running,\n"
  "                       stat.events / seconds,\n"
  "                       (stat.reads + stat.writes + stat.other) / seconds,\n"
  "                       stat.reads / seconds,\n"
  "                       stat.writes / seconds,\n"
  "                       stat.other / seconds,\n"
  "                       sysbench.opt.percentile,\n"
  "                       stat.latency_pct * 1000,\n"
  "                       stat.errors / seconds,\n"
  "                       stat.reconnects / seconds\n"
  "   ))\n"
  "end\n"
;
size_t sysbench_lua_len = sizeof(sysbench_lua) - 1;