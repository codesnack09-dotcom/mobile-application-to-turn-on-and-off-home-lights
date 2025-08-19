import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'firebase_options.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(const MultiLampApp());
}

class MultiLampApp extends StatelessWidget {
  const MultiLampApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Multi Lamp Controller',
      theme: ThemeData(
        colorSchemeSeed: Colors.teal,
        brightness: Brightness.dark,
        useMaterial3: true,
      ),
      home: const DevicesPage(),
    );
  }
}

class DevicesPage extends StatefulWidget {
  const DevicesPage({super.key});

  @override
  State<DevicesPage> createState() => _DevicesPageState();
}

class _DevicesPageState extends State<DevicesPage> {
  final DatabaseReference devicesRef =
      FirebaseDatabase.instance.ref('devices');

  Future<void> _toggle(String deviceId, bool on) async {
    await devicesRef.child(deviceId).child('state').set(on ? 'ON' : 'OFF');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Multi Lamp Controller'),
      ),
      body: StreamBuilder<DatabaseEvent>(
        stream: devicesRef.onValue,
        builder: (context, snapshot) {
          if (snapshot.hasError) {
            return Center(child: Text('Error: ${snapshot.error}'));
          }
          if (!snapshot.hasData ||
              (snapshot.data!.snapshot.value == null)) {
            return const Center(child: Text('No devices found'));
          }

          final map = Map<String, dynamic>.from(
              snapshot.data!.snapshot.value as Map);

          final entries = map.entries.toList()
            ..sort((a, b) => a.key.compareTo(b.key));

          return ListView.builder(
            itemCount: entries.length,
            itemBuilder: (context, i) {
              final id = entries[i].key;
              final data = Map<String, dynamic>.from(entries[i].value);
              final name = (data['name'] ?? id).toString();
              final state = (data['state'] ?? 'OFF').toString().toUpperCase();
              final status = (data['status'] ?? 'UNKNOWN').toString().toUpperCase();
              final isOn = state == 'ON';

              return Card(
                margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                child: ListTile(
                  leading: Icon(
                    isOn ? Icons.lightbulb : Icons.lightbulb_outline,
                    size: 32,
                  ),
                  title: Text(name, style: const TextStyle(fontSize: 18)),
                  subtitle: Text('ID: $id   •   Device status: $status'),
                  trailing: Switch.adaptive(
                    value: isOn,
                    onChanged: (v) => _toggle(id, v),
                  ),
                ),
              );
            },
          );
        },
      ),
      floatingActionButton: FloatingActionButton.extended(
        onPressed: () async {
          // Demo: tambah dummy device di DB (untuk uji tampilan)
          final newId = 'lamp${DateTime.now().millisecondsSinceEpoch % 10000}';
          await devicesRef.child(newId).set({
            'name': 'New Lamp',
            'state': 'OFF',
            'status': 'OFF',
          });
        },
        label: const Text('Add device (demo)'),
        icon: const Icon(Icons.add),
      ),
    );
  }
}
